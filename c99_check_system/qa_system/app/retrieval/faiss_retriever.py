"""
app/retrieval/faiss_retriever.py — FAISS 法律法规向量检索

加载 data/faiss/index.faiss + meta.json，按语义相似度召回条文分段。
同学交付同结构索引目录后，覆盖 FAISS_INDEX_DIR 即可切换真实数据。
"""

from __future__ import annotations

import json
import logging
import threading
from pathlib import Path
from typing import Any, Optional

from config import settings

logger = logging.getLogger(__name__)


class FaissRetriever:
    """线程安全的 FAISS 检索单例。"""

    _instance: Optional["FaissRetriever"] = None
    _lock = threading.Lock()

    def __init__(self, index_dir: Optional[str] = None):
        self.index_dir = Path(index_dir or settings.FAISS_INDEX_DIR)
        self.model_name = settings.EMBEDDING_MODEL
        self._index = None
        self._meta: list[dict[str, Any]] = []
        self._embedder = None
        self._loaded = False
        self._embed_info: dict[str, Any] = {}

    @classmethod
    def get_instance(cls) -> "FaissRetriever":
        if cls._instance is None:
            with cls._lock:
                if cls._instance is None:
                    cls._instance = FaissRetriever()
        return cls._instance

    @classmethod
    def reset_instance(cls) -> None:
        with cls._lock:
            cls._instance = None

    @property
    def is_ready(self) -> bool:
        return self._loaded and self._index is not None and bool(self._meta)

    def load(self) -> bool:
        """加载索引；成功返回 True。"""
        index_path = self.index_dir / "index.faiss"
        meta_path = self.index_dir / "meta.json"
        info_path = self.index_dir / "embed_info.json"

        if not index_path.exists() or not meta_path.exists():
            logger.warning(
                "[FaissRetriever] Index not found under %s "
                "(need index.faiss + meta.json). Run: python scripts/build_faiss_index.py",
                self.index_dir,
            )
            self._loaded = False
            return False

        try:
            import faiss
            from app.retrieval.embedder import TextEmbedder

            logger.info("[FaissRetriever] Loading index from %s", self.index_dir)
            import numpy as np

            raw = np.frombuffer(index_path.read_bytes(), dtype="uint8")
            try:
                self._index = faiss.deserialize_index(raw)
            except Exception:
                # 兼容 faiss.write_index 直接写出的文件（无中文路径时）
                self._index = faiss.read_index(str(index_path))
            with open(meta_path, "r", encoding="utf-8") as f:
                self._meta = json.load(f)

            if info_path.exists():
                with open(info_path, "r", encoding="utf-8") as f:
                    self._embed_info = json.load(f)
            else:
                self._embed_info = {}

            self._embedder = TextEmbedder(self.model_name)
            # 若索引用 hashing 构建，强制 hashing，避免维度不一致
            preferred = (self._embed_info.get("backend") or "").strip()
            if preferred == "hashing":
                self._embedder._backend = "hashing"
                self._embedder.dim = int(self._embed_info.get("dim") or self._embedder.dim)
                logger.info("[FaissRetriever] Using hashing embedder to match index")
            else:
                self._embedder.load()

            self._loaded = True
            logger.info(
                "[FaissRetriever] Ready: %d vectors, dim=%s, embed=%s",
                len(self._meta),
                self._index.d if self._index else "?",
                self._embedder.backend,
            )
            return True
        except Exception as e:
            logger.error("[FaissRetriever] Failed to load: %s", e, exc_info=True)
            self._loaded = False
            return False

    def search(self, query: str, top_k: Optional[int] = None) -> list[dict[str, Any]]:
        """
        语义检索。返回列表项含：
          chunk_id, law_name, article, text, source, score
        """
        if not query or not query.strip():
            return []

        if not self.is_ready:
            if not self.load():
                return []

        import numpy as np

        k = top_k if top_k is not None else settings.RAG_TOP_K
        k = max(1, min(k, len(self._meta)))

        vector = self._embedder.encode([query.strip()])
        vector = np.asarray(vector, dtype="float32")
        scores, indices = self._index.search(vector, k)

        results: list[dict[str, Any]] = []
        for score, idx in zip(scores[0], indices[0]):
            if idx < 0 or idx >= len(self._meta):
                continue
            item = dict(self._meta[idx])
            item["score"] = float(score)
            results.append(item)
        return results

    def get_by_article(self, article: str) -> list[dict[str, Any]]:
        """按条款编号精确匹配（如 "6.5.2.2"），用于编号输入路径。"""
        if not self.is_ready:
            if not self.load():
                return []
        target = (article or "").strip()
        if not target:
            return []
        results: list[dict[str, Any]] = []
        for item in self._meta:
            if (item.get("article") or "").strip() == target:
                out = dict(item)
                out["score"] = 1.0  # 精确匹配给满分
                results.append(out)
        return results


def get_faiss_retriever() -> FaissRetriever:
    return FaissRetriever.get_instance()
