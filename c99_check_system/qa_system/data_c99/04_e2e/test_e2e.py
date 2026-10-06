#!/usr/bin/env python3
"""
test_e2e.py — 端到端测试（留痕）

验证完整链路：输入（编号 / 中文描述）→ 检索 → LLM 生成 C 测试代码。

前置：后端已启动（uvicorn main:app --port 8000）

运行：python data_c99/04_e2e/test_e2e.py
输出：完整结果保存到 data_c99/04_e2e/output.txt（留痕）
"""

import json
import urllib.request
from pathlib import Path

BASE = "http://localhost:8000/api/qa/ask"
OUT = Path(__file__).with_name("output.txt")


def ask(q: str, timeout: int = 300) -> dict:
    req = urllib.request.Request(
        BASE,
        data=json.dumps({"question": q}).encode("utf-8"),
        headers={"Content-Type": "application/json"},
    )
    return json.loads(urllib.request.urlopen(req, timeout=timeout).read())


def main() -> None:
    lines: list[str] = []
    cases = ["6.5.2.2", "函数调用的规则", "6.5.6"]

    for q in cases:
        lines.append("=" * 70)
        lines.append(f"输入: {q}")
        try:
            r = ask(q)
            lines.append(f"intent: {r.get('intent')}")
            src = [
                (s.get("object_id"), s.get("museum_name"))
                for s in r.get("sources", [])
            ]
            lines.append(f"sources: {src}")
            ans = r.get("answer") or ""
            lines.append(f"answer ({len(ans)} chars):")
            lines.append(ans)
        except Exception as e:
            lines.append(f"ERROR: {e}")
        lines.append("")

    text = "\n".join(lines)
    OUT.write_text(text, encoding="utf-8")
    print(f"端到端测试完成，结果已保存到 {OUT}")
    print(f"共 {len(cases)} 个用例，输出 {len(text)} 字符")


if __name__ == "__main__":
    main()
