#!/usr/bin/env python3
"""gen_coverage.py — 批量生成指定章节的测试代码，供「读代码评审覆盖率」用。

用法（后端已启动 :8000）：
  python scripts/gen_coverage.py --pattern '6\.5(\.\d+)*' --out _coverage/6.5
  python scripts/gen_coverage.py --pattern '7\.4(\.\d+)*' --out _coverage/7.4 --workers 8

对每个条款调用 /api/qa/ask 生成代码，抽取 ```c 块，存 <out>/<article>.c，
并把 {article, text, code, status} 写入 <out>/manifest.jsonl。
支持 --workers 并发（默认 8 路）与 --timeout 硬超时（默认 90s，卡死快速失败）。
"""
import argparse
import json
import re
import time
import urllib.request
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path

BASE = "http://localhost:8000/api/qa/ask"


def ask(q: str, timeout: int = 90) -> str:
    req = urllib.request.Request(
        BASE,
        data=json.dumps({"question": q}).encode("utf-8"),
        headers={"Content-Type": "application/json"},
    )
    r = json.loads(urllib.request.urlopen(req, timeout=timeout).read())
    ans = r.get("answer") or ""
    m = re.search(r"```c\s*\n(.*?)```", ans, re.S)
    return m.group(1).strip() if m else ""


def gen_one(c: dict, retries: int, timeout: int):
    a = c["article"]
    code = ""
    last_err = ""
    for _ in range(retries + 1):
        try:
            code = ask(a, timeout)
            if code:
                break
        except Exception as e:
            last_err = str(e)
            code = ""
        time.sleep(1)
    status = "ok" if code else (f"ERR:{last_err}" if last_err else "EMPTY")
    return a, c["text"], code, status


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--chunks", default="data_c99/chunks.jsonl")
    ap.add_argument("--pattern", default=r"6\.5(\.\d+)*")
    ap.add_argument("--out", default="_coverage/6.5")
    ap.add_argument("--retries", type=int, default=3)
    ap.add_argument("--workers", type=int, default=8)
    ap.add_argument("--timeout", type=int, default=90)
    ap.add_argument("--min-text", type=int, default=0, help="跳过正文少于 N 字符的纯标题条款")
    ap.add_argument("--exclude", type=str, default=None, help="正则，跳过条款号匹配的条款（如 7\\.26 未来方向）")
    args = ap.parse_args()

    pat = re.compile(args.pattern)
    clauses = [json.loads(l) for l in open(args.chunks, encoding="utf-8") if pat.fullmatch(json.loads(l)["article"])]
    if args.min_text:
        clauses = [c for c in clauses if len(c["text"]) >= args.min_text]
    if args.exclude:
        ex = re.compile(args.exclude)
        clauses = [c for c in clauses if not ex.search(c["article"])]
    outdir = Path(args.out)
    outdir.mkdir(parents=True, exist_ok=True)

    print(f"{len(clauses)} 条条款, {args.workers} 路并发, 超时 {args.timeout}s, 重试 {args.retries} 次", flush=True)

    manifest = []
    done = 0
    t0 = time.time()
    with ThreadPoolExecutor(max_workers=args.workers) as ex:
        futures = {ex.submit(gen_one, c, args.retries, args.timeout): c for c in clauses}
        for fut in as_completed(futures):
            a, text, code, status = fut.result()
            (outdir / f"{a}.c").write_text(code, encoding="utf-8")
            manifest.append({"article": a, "text": text, "code": code, "status": status})
            done += 1
            print(f"[{done}/{len(clauses)}] {a}: {len(code)} chars ({status})", flush=True)

    order = {c["article"]: i for i, c in enumerate(clauses)}
    manifest.sort(key=lambda r: order[r["article"]])
    (outdir / "manifest.jsonl").write_text(
        "\n".join(json.dumps(r, ensure_ascii=False) for r in manifest), encoding="utf-8"
    )
    dt = time.time() - t0
    ok = sum(1 for r in manifest if r["status"] == "ok")
    print(f"DONE {len(clauses)} 条 -> {outdir}  成功 {ok}  耗时 {dt/60:.1f} 分钟", flush=True)


if __name__ == "__main__":
    main()
