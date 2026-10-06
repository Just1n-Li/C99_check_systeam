"""
app/retrieval/legal_data_loader.py — 加载清洗同学交付的法规数据

约定目录（可用 LAW_DATA_DIR / --input 覆盖）：
  data_cleaned/chunks.jsonl  — 按法条切分的检索块（主输入）
  data_cleaned/laws.json     — 法规全文元数据（可选，用于补全来源）

chunks.jsonl 每行字段（清洗侧交付）：
  id, text, law_name, article, status, source_url, source_file

规范化后供 FAISS 使用的字段：
  chunk_id, law_name, article, text, source, status, source_file, law_id
"""

from __future__ import annotations

import json
import logging
import re
from pathlib import Path
from typing import Any, Optional

logger = logging.getLogger(__name__)

# 列表页 / 目录页等不应进入检索索引
_SKIP_LAW_NAMES = {"法规列表", "目录", "列表"}
_NOISE_PATTERNS = [
    re.compile(r"收藏\s*[|｜]\s*分享\s*[|｜]\s*打印"),
    re.compile(r"^\s*收藏\s*$", re.M),
]


def _clean_text(text: str) -> str:
    t = (text or "").strip()
    for pat in _NOISE_PATTERNS:
        t = pat.sub("", t)
    return t.strip()


def load_laws_index(laws_path: Path) -> dict[str, dict[str, Any]]:
    """laws.json → {law_id: law_record}"""
    if not laws_path.exists():
        return {}
    try:
        data = json.loads(laws_path.read_text(encoding="utf-8"))
    except Exception as e:
        logger.warning("[LegalData] failed to read %s: %s", laws_path, e)
        return {}
    if not isinstance(data, list):
        return {}
    return {str(item.get("id")): item for item in data if isinstance(item, dict) and item.get("id")}


def _normalize_chunk(
    raw: dict[str, Any],
    index: int,
    laws_by_id: Optional[dict[str, dict]] = None,
) -> Optional[dict[str, Any]]:
    """将清洗侧字段映射为内部统一结构。"""
    text = _clean_text(raw.get("text") or "")
    if not text:
        return None

    law_name = (raw.get("law_name") or raw.get("title") or "").strip()
    if law_name in _SKIP_LAW_NAMES:
        return None

    article = raw.get("article")
    if article is not None:
        article = str(article).strip() or None

    # 列表页常无 article 且正文极短——跳过无正文实质内容的块
    chunk_id = (
        raw.get("id")
        or raw.get("chunk_id")
        or f"chunk_{index}"
    )
    chunk_id = str(chunk_id).strip()

    source = (
        raw.get("source_url")
        or raw.get("source")
        or raw.get("detail_url")
        or ""
    ).strip()

    law_id = None
    if "-" in chunk_id:
        # 清洗侧 id 形如 "{doc_hash}-{n}" 或 "{doc_hash}-full"
        law_id = chunk_id.rsplit("-", 1)[0]

    status = raw.get("status")
    source_file = (raw.get("source_file") or "").strip()

    # 用 laws.json 补全空 source / status
    if laws_by_id and law_id and law_id in laws_by_id:
        law = laws_by_id[law_id]
        if not source:
            source = (law.get("source_url") or "").strip()
        if not status:
            status = law.get("status")
        if not law_name:
            law_name = (law.get("title") or "").strip()

    if not law_name:
        return None

    # 跳过明显是目录/全文占位且无条文的块
    if not article and ("列表" in text[:40] or len(text) < 20):
        return None

    return {
        "chunk_id": chunk_id,
        "law_id": law_id or "",
        "law_name": law_name,
        "article": article or "",
        "text": text,
        "source": source,
        "status": status,
        "source_file": source_file,
    }


def load_chunks_from_jsonl(
    path: Path,
    laws_by_id: Optional[dict[str, dict]] = None,
) -> list[dict[str, Any]]:
    chunks: list[dict[str, Any]] = []
    with open(path, "r", encoding="utf-8") as f:
        for i, line in enumerate(f):
            line = line.strip()
            if not line:
                continue
            try:
                raw = json.loads(line)
            except json.JSONDecodeError as e:
                logger.warning("[LegalData] skip bad jsonl line %d: %s", i + 1, e)
                continue
            if not isinstance(raw, dict):
                continue
            norm = _normalize_chunk(raw, i, laws_by_id)
            if norm:
                chunks.append(norm)
    return chunks


def load_chunks_from_json_array(
    path: Path,
    laws_by_id: Optional[dict[str, dict]] = None,
) -> list[dict[str, Any]]:
    data = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(data, list):
        raise ValueError(f"Expected JSON array in {path}")
    chunks: list[dict[str, Any]] = []
    for i, raw in enumerate(data):
        if not isinstance(raw, dict):
            continue
        norm = _normalize_chunk(raw, i, laws_by_id)
        if norm:
            chunks.append(norm)
    return chunks


def load_legal_chunks(
    chunks_path: Path,
    laws_path: Optional[Path] = None,
) -> list[dict[str, Any]]:
    """
    加载并规范化检索块。

    支持：
      - *.jsonl（清洗同学交付格式）
      - *.json 数组（旧样例 sample_chunks.json）
    """
    if not chunks_path.exists():
        raise FileNotFoundError(f"Chunks file not found: {chunks_path}")

    laws_by_id = load_laws_index(laws_path) if laws_path else {}

    suffix = chunks_path.suffix.lower()
    if suffix == ".jsonl":
        chunks = load_chunks_from_jsonl(chunks_path, laws_by_id)
    else:
        chunks = load_chunks_from_json_array(chunks_path, laws_by_id)

    logger.info(
        "[LegalData] loaded %d chunks from %s (laws meta: %d)",
        len(chunks),
        chunks_path,
        len(laws_by_id),
    )
    return chunks


def default_cleaned_dir(base_dir: Path) -> Path:
    return base_dir / "data_cleaned"
