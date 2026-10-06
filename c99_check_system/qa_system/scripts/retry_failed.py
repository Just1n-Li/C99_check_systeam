#!/usr/bin/env python3
"""retry_failed.py — 重试 manifest 里生成失败的条款（分批重试，避免后端劣化）。

用法：
  python scripts/retry_failed.py <输出目录> [每批条数] [并发数]

每批处理完即写回 manifest + .c，方便分批 + 批次间重启后端。
"""
import json
import re
import sys
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


def retry_one(r: dict, retries: int = 2) -> tuple:
    a = r["article"]
    code = ""
    status = "ERR"
    for _ in range(retries + 1):
        try:
            code = ask(a)
            if code:
                status = "ok"
                break
        except Exception:
            code = ""
        time.sleep(1)
    return a, r["text"], code, status


def main() -> None:
    outdir = Path(sys.argv[1])
    limit = int(sys.argv[2]) if len(sys.argv) > 2 else 0
    workers = int(sys.argv[3]) if len(sys.argv) > 3 else 4

    mp = outdir / "manifest.jsonl"
    rows = [json.loads(l) for l in open(mp, encoding="utf-8")]
    failed = [r for r in rows if r["status"] != "ok"]
    if limit:
        failed = failed[:limit]

    print(f"重试 {len(failed)} 条（{workers} 路并发）", flush=True)
    fixed = 0
    t0 = time.time()
    with ThreadPoolExecutor(max_workers=workers) as ex:
        futures = {ex.submit(retry_one, r): r for r in failed}
        for fut in as_completed(futures):
            a, text, code, status = fut.result()
            for r in rows:
                if r["article"] == a:
                    r["code"] = code
                    r["status"] = status
                    break
            if status == "ok":
                (outdir / f"{a}.c").write_text(code, encoding="utf-8")
                fixed += 1
            print(f"{a}: {'ok' if status == 'ok' else 'FAIL'} ({len(code)}字)", flush=True)

    open(mp, "w", encoding="utf-8").write(
        "\n".join(json.dumps(r, ensure_ascii=False) for r in rows)
    )
    print(f"完成：修复 {fixed}/{len(failed)}  耗时 {time.time()-t0:.0f}s", flush=True)


if __name__ == "__main__":
    main()
