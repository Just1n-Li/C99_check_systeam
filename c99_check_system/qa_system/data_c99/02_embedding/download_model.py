#!/usr/bin/env python3
"""
download_model.py — 下载并验证 embedding 模型（向量化步骤留痕）

模型：sentence-transformers/paraphrase-multilingual-MiniLM-L12-v2
  - 多语言语义嵌入，支持中英跨语言匹配（中文描述 → 英文 C99 条款）
  - 输出维度 384
下载源：HF_ENDPOINT=https://hf-mirror.com（国内镜像，模型缓存到本地 HF hub 缓存）

运行：python data_c99/02_embedding/download_model.py
"""

import os

# 国内镜像（优先级低于已设置的环境变量）
os.environ.setdefault("HF_ENDPOINT", "https://hf-mirror.com")

import numpy as np
from sentence_transformers import SentenceTransformer

MODEL = "sentence-transformers/paraphrase-multilingual-MiniLM-L12-v2"


def main() -> None:
    print(f"loading model: {MODEL} ...")
    model = SentenceTransformer(MODEL)

    # 跨语言验证：中文描述 vs 英文条款标题
    zh = ["函数调用的规则", "加法运算符", "类型转换", "位运算"]
    en = ["Function calls", "Additive operators", "Cast operators", "Bitwise shift operators"]

    zh_vec = model.encode(zh, normalize_embeddings=True)
    en_vec = model.encode(en, normalize_embeddings=True)

    sim = np.dot(zh_vec, en_vec.T)

    print(f"model : {MODEL}")
    print(f"dim   : {zh_vec.shape[-1]}")
    print("cross-lingual similarity (rows=中文, cols=英文):")
    for i, z in enumerate(zh):
        best = int(np.argmax(sim[i]))
        print(f"  「{z}」 → 最相似「{en[best]}」({sim[i][best]:.3f})")
        print(f"     相似度: " + ", ".join(f"{en[j]}={sim[i][j]:.3f}" for j in range(len(en))))


if __name__ == "__main__":
    main()
