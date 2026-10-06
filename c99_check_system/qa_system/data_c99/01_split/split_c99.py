#!/usr/bin/env python3
"""
split_c99.py — 从 C99 标准 HTML（N1256 / TC3）切分 §6.1–§7.26（第 6 章语言 + 第 7 章库）

数据来源（留痕）：
  ISO/IEC 9899:1999 + TC3 (N1256) 的 HTML 渲染版
  https://port70.net/~nsz/c/c99/n1256.html
  本地留存：data_c99/00_raw/n1256_c99_tc3.html

输入：data_c99/00_raw/n1256_c99_tc3.html
输出：data_c99/chunks.jsonl（与现有法律 chunks.jsonl 字段兼容）

切分粒度：每个带独立编号的条款（clause）一条 chunk，正文保留段落编号与
Constraints / Semantics / Footnotes / EXAMPLE 等结构。
"""

import html
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]  # data_c99
RAW = ROOT / "00_raw" / "n1256_c99_tc3.html"
OUT = ROOT / "chunks.jsonl"

LAW_NAME = "ISO/IEC 9899:1999 (C99)"
SOURCE_URL_BASE = "https://port70.net/~nsz/c/c99/n1256.html"

# 条款标题：<h3/4/5><a name="6.5..." href="...">标题</a></h3/4/5>
# 覆盖第 6 章 6.1~6.11 与第 7 章 7.1~7.26 的全部编号条款；章节标题「6」「7」是 h2，不会命中
CLAUSE_RE = re.compile(r'<h[3-5]><a name="((?:6|7)\.\d+(?:\.\d+)*)"[^>]*>([^<]*)</a></h[3-5]>')

# 段落切分：每个 <p> 到下一个 <p> / <h> 为止
PARA_RE = re.compile(r"<p>(.*?)(?=<p>|<h[3-5]>|$)", re.S)

TAG_RE = re.compile(r"<[^>]+>")
WS_RE = re.compile(r"\s+")


def strip_html(s: str) -> str:
    # 去掉脚注引用（<sup>80)</sup> 等），保留正文与交叉引用
    s = re.sub(r"<sup>.*?</sup>", "", s, flags=re.S)
    s = re.sub(r"<sub>.*?</sub>", "", s, flags=re.S)
    s = TAG_RE.sub("", s)
    s = html.unescape(s)
    return WS_RE.sub(" ", s).strip()


def process_paragraph(para_html: str):
    """规范化单个段落，返回文本或 None（表示跳过）。"""
    # 页面末尾的"返回目录"导航链接
    if 'href="#Contents"' in para_html:
        return None

    # 带段号的正文段落：<a name="...pN"><small>N</small></a> 之后是正文
    m = re.search(r'<a name="[^"]*p(\d+)"[^>]*><small>\d+</small></a>', para_html)
    if m:
        pn = m.group(1)
        text = strip_html(para_html[m.end():])
        return f"[{pn}] {text}" if text else None

    # 无段号段落：小节标题 / 脚注 / Forward references
    text = strip_html(para_html)
    if not text:
        return None
    # 脚注（"97) ..."）或含完整句子的引用 → 原样保留
    if re.match(r"^\d+\)", text) or text.endswith("."):
        return text
    # 小节标题（Constraints / Semantics / Footnotes / Description 等）
    return f"{text}:"


def main() -> None:
    raw = RAW.read_text(encoding="utf-8")

    # 定位 §6.1~§7.26：从 6.1 标题到附录 A.1 之前（覆盖第 6 章语言 + 第 7 章库）
    start = raw.index('<h3><a name="6.1"')
    end = raw.index('<h3><a name="A.1"', start)
    section = raw[start:end]

    clauses = list(CLAUSE_RE.finditer(section))
    chunks = []

    for i, m in enumerate(clauses):
        num = m.group(1)
        title = html.unescape(m.group(2).strip())
        english_title = re.sub(r"^" + re.escape(num) + r"\s*", "", title).strip()

        body_start = m.end()
        body_end = clauses[i + 1].start() if i + 1 < len(clauses) else len(section)
        body_html = section[body_start:body_end]

        paragraphs = []
        for p in PARA_RE.finditer(body_html):
            out = process_paragraph(p.group(1))
            if out:
                paragraphs.append(out)

        body = "\n".join(paragraphs)
        full_text = english_title + ("\n" + body if body else "")

        chunks.append({
            "id": f"c99-{num}",
            "text": full_text,
            "law_name": LAW_NAME,
            "article": num,
            "status": None,
            "source_url": f"{SOURCE_URL_BASE}#{num}",
            "source_file": "n1256_c99_tc3.html",
        })

    OUT.parent.mkdir(parents=True, exist_ok=True)
    with open(OUT, "w", encoding="utf-8") as f:
        for c in chunks:
            f.write(json.dumps(c, ensure_ascii=False) + "\n")

    print(f"split done: {len(chunks)} clauses -> {OUT}")
    for c in chunks:
        preview = c["text"].split("\n")[0][:50]
        print(f"  {c['article']:<10} {preview}")


if __name__ == "__main__":
    main()
