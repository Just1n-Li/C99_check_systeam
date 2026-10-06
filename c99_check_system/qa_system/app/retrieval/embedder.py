"""
app/retrieval/embedder.py — 文本向量化

优先 sentence-transformers；不可用时回退到纯 numpy 字符 n-gram 哈希向量，
以便在 Windows 长路径等环境下仍能构建/加载 FAISS 假数据索引。
"""

from __future__ import annotations

import hashlib
import logging
import os
from typing import Optional

import numpy as np

from config import settings

logger = logging.getLogger(__name__)

# 轻量回退向量维度（与 FAISS IndexFlatIP 配合，需 L2 归一化）
HASH_EMBED_DIM = 384


class TextEmbedder:
    def __init__(self, model_name: Optional[str] = None):
        self.model_name = model_name or settings.EMBEDDING_MODEL
        self._backend = "none"
        self._model = None
        self.dim = HASH_EMBED_DIM

    def load(self) -> str:
        """加载后端，返回 backend 名称：sentence-transformers | hashing。"""
        try:
            # 国内镜像，避免直连 huggingface.co 被拒（模型已在本地缓存时秒加载）
            os.environ.setdefault("HF_ENDPOINT", "https://hf-mirror.com")
            # 模型已缓存，离线加载跳过 HF 连接检查，避免每次启动重试超时
            os.environ.setdefault("HF_HUB_OFFLINE", "1")
            from sentence_transformers import SentenceTransformer

            self._model = SentenceTransformer(self.model_name, local_files_only=True)
            self._backend = "sentence-transformers"
            # 探测维度
            probe = self._model.encode(["test"], normalize_embeddings=True)
            self.dim = int(np.asarray(probe).shape[-1])
            logger.info("[Embedder] sentence-transformers (%s), dim=%d", self.model_name, self.dim)
            return self._backend
        except Exception as e:
            logger.warning(
                "[Embedder] sentence-transformers unavailable (%s), using hashing fallback",
                e,
            )
            self._backend = "hashing"
            self.dim = HASH_EMBED_DIM
            return self._backend

    @property
    def backend(self) -> str:
        return self._backend

    def encode(self, texts: list[str]) -> np.ndarray:
        if self._backend == "none":
            self.load()
        if self._backend == "sentence-transformers" and self._model is not None:
            vectors = self._model.encode(texts, show_progress_bar=False, normalize_embeddings=True)
            return np.asarray(vectors, dtype="float32")
        return _hash_embed_batch(texts, self.dim)


def _hash_embed_batch(texts: list[str], dim: int) -> np.ndarray:
    out = np.zeros((len(texts), dim), dtype="float32")
    for i, text in enumerate(texts):
        out[i] = _hash_embed(text or "", dim)
    # L2 normalize for inner-product search
    norms = np.linalg.norm(out, axis=1, keepdims=True)
    norms = np.maximum(norms, 1e-8)
    return out / norms


def _hash_embed(text: str, dim: int) -> np.ndarray:
    """字符 bigram 哈希向量（确定性，无需模型权重）。"""
    vec = np.zeros(dim, dtype="float32")
    s = text.strip().lower()
    if not s:
        return vec
    # unigrams + bigrams
    tokens = list(s) + [s[j : j + 2] for j in range(len(s) - 1)]
    for tok in tokens:
        h = hashlib.md5(tok.encode("utf-8")).hexdigest()
        idx = int(h[:8], 16) % dim
        sign = 1.0 if (int(h[8:16], 16) % 2 == 0) else -1.0
        vec[idx] += sign
    return vec
