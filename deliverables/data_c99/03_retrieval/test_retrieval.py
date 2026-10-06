#!/usr/bin/env python3
"""
test_retrieval.py — 检索效果实测（留痕）

验证两条路径：
  1. 编号精确匹配（get_by_article）：输入条款编号直接命中
  2. 语义检索（search）：中文描述跨语言匹配英文条款

运行：python data_c99/03_retrieval/test_retrieval.py
"""

import sys
from pathlib import Path

# 让脚本能 import 项目根（qa_system 目录）下的 app.*
sys.path.insert(0, str(Path(__file__).resolve().parents[2]))

from app.retrieval.faiss_retriever import FaissRetriever


def main() -> None:
    r = FaissRetriever()
    if not r.load():
        print("索引加载失败")
        return

    print(f"索引就绪：{len(r._meta)} 个条款，dim={r._index.d if r._index else '?'}，backend={r._embedder.backend}")

    print("\n=== ① 编号精确匹配：6.5.2.2 ===")
    for h in r.get_by_article("6.5.2.2"):
        first_line = h["text"].split("\n")[0]
        print(f"  [{h['article']}] {first_line}  (score={h['score']})")

    print("\n=== ② 语义检索：「函数调用的规则」 top3 ===")
    for h in r.search("函数调用的规则", top_k=3):
        first_line = h["text"].split("\n")[0]
        print(f"  [{h['article']}] {first_line}  (score={h['score']:.3f})")

    print("\n=== ③ 语义检索：「加法运算符」 top3 ===")
    for h in r.search("加法运算符", top_k=3):
        first_line = h["text"].split("\n")[0]
        print(f"  [{h['article']}] {first_line}  (score={h['score']:.3f})")

    print("\n=== ④ 语义检索：「sizeof」 top3 ===")
    for h in r.search("sizeof 运算符如何取结构体大小", top_k=3):
        first_line = h["text"].split("\n")[0]
        print(f"  [{h['article']}] {first_line}  (score={h['score']:.3f})")


if __name__ == "__main__":
    main()
