#!/usr/bin/env python3
"""retry_stubborn.py — 对生成失败的顽固条款做降级重试。

降级手段：
1. 截断过长的条款正文（>2500 字符只留开头，减小推理负担、避免模型卡死）。
2. 直连 LLM（非流式、硬超时 90s），超时快速失败再重试，而不是像后端那样无限挂起。

用法（后端可不必运行；从 manifest 里找空代码的条款）：
  python scripts/retry_stubborn.py <manifest_dir>
"""
import sys
import json
import re
import time
import pathlib

BASE = pathlib.Path(r"D:\C99_check_system\legal_qa_system\qa_system")
sys.path.insert(0, str(BASE))

from config import settings
from app.retrieval.llm_generator import ANSWER_SYSTEM_PROMPT
from openai import OpenAI

MAXLEN = 2500
TIMEOUT = 90
ATTEMPTS = 3


def truncate(text: str) -> str:
    if len(text) <= MAXLEN:
        return text
    cut = text[:MAXLEN]
    nl = cut.rfind("\n")
    return cut[:nl] if nl > 0 else cut


def main() -> None:
    ch6 = pathlib.Path(sys.argv[1]) if len(sys.argv) > 1 else BASE / "_stubborn"
    mp = ch6 / "manifest.jsonl"
    rows = [json.loads(l) for l in open(mp, encoding="utf-8")]
    stubborn = [r for r in rows if not r.get("code")]
    print(f"顽固条款 {len(stubborn)}: {[r['article'] for r in stubborn]}", flush=True)

    client = OpenAI(
        api_key=settings.resolved_llm_api_key(),
        base_url=settings.resolved_llm_base_url(),
        timeout=TIMEOUT,
        max_retries=0,
    )

    fixed = 0
    for r in stubborn:
        a = r["article"]
        text = truncate(r["text"])
        user = (
            f"C99 条款：{a}\n\n条款原文（C99 标准）：\n{text}\n\n"
            f"请根据以上条款，生成用于测试编译器是否符合该条款的 C 测试程序："
        )
        code = ""
        for attempt in range(ATTEMPTS):
            try:
                resp = client.chat.completions.create(
                    model=settings.LLM_MODEL_NAME,
                    messages=[
                        {"role": "system", "content": ANSWER_SYSTEM_PROMPT},
                        {"role": "user", "content": user},
                    ],
                    temperature=0.1,
                    max_tokens=8192,
                )
                ans = resp.choices[0].message.content or ""
                m = re.search(r"```c\s*\n(.*?)```", ans, re.S)
                if m and m.group(1).strip():
                    code = m.group(1).strip()
                    break
            except Exception as e:
                print(f"  {a} 第{attempt+1}次: {type(e).__name__}", flush=True)
            time.sleep(1)

        if code:
            r["code"] = code
            r["status"] = "ok"
            (ch6 / f"{a}.c").write_text(code, encoding="utf-8")
            fixed += 1
            print(f"{a}: ok ({len(code)} chars)", flush=True)
        else:
            print(f"{a}: STILL FAIL", flush=True)

    open(mp, "w", encoding="utf-8").write(
        "\n".join(json.dumps(r, ensure_ascii=False) for r in rows)
    )
    print(f"降级完成: 修复 {fixed}/{len(stubborn)}", flush=True)


if __name__ == "__main__":
    main()
