"""
从检索结果中提取溯源信息。

法律法规场景：优先使用 FAISS hits → sources（museum=法律名, object_id=条文号）。
遗留：MySQL/Neo4j 文物溯源逻辑保留在下方，默认问答链路不再调用。
"""

from __future__ import annotations

import json
import logging
from typing import Any, Optional


def extract_sources_from_hits(hits: list[dict]) -> list[dict]:
    """FAISS hits → 会话/前端兼容的 sources 列表。"""
    from app.agents.summarizer_agent import chunks_to_sources
    return chunks_to_sources(hits or [])


# ── 以下为遗留文物溯源（ENABLE_MYSQL/NEO4J 旧链路）─────────────

import re
import asyncio

ARTIFACT_URI_RE = re.compile(r"entity:artifact:(\d+):([^\s\"\'\],}]+)")
MUSEUM_URI_RE = re.compile(r"entity:museum:(\d+)")
DATA_QUERY_TOOLS = frozenset({"execute_sql", "query_neo4j", "search_legal_docs"})
MAX_SOURCES = 25


def _source_dedup_key(row: dict) -> str:
    url = (row.get("url") or row.get("detail_url") or "").strip().lower()
    if url:
        return f"url:{url}"
    object_id = (row.get("object_id") or "").strip()
    if object_id:
        return f"oid:{object_id}"
    return ""


def _append_source_if_new(seen: set[str], sources: list[dict], row: dict) -> None:
    key = _source_dedup_key(row)
    if not key or key in seen:
        return
    url = (row.get("url") or row.get("detail_url") or "").strip()
    if not url:
        return
    seen.add(key)
    sources.append({
        k: v for k, v in row.items()
        if not k.startswith("_") and v is not None and k != "detail_url"
    })
    if "url" not in sources[-1]:
        sources[-1]["url"] = url


QuerySnapshot = tuple[str, str, Optional[int], Optional[str]]


def _unpack_snapshot(item: QuerySnapshot | tuple) -> tuple[str, str, Optional[int], Optional[str]]:
    if len(item) >= 4:
        return item[0], item[1], item[2], item[3]
    if len(item) == 3:
        return item[0], item[1], item[2], None
    return item[0], item[1], None, None


def _records_from_tool_output(content: str) -> list[dict]:
    if not content:
        return []
    try:
        data = json.loads(content)
    except (json.JSONDecodeError, TypeError):
        return []
    if isinstance(data, dict):
        if data.get("error"):
            return []
        if "hits" in data and isinstance(data["hits"], list):
            return [h for h in data["hits"] if isinstance(h, dict)]
        return [data]
    if isinstance(data, list):
        return [r for r in data if isinstance(r, dict) and not r.get("error")]
    return []


def _museum_id_from_text(text: str) -> Optional[int]:
    if not text:
        return None
    found: Optional[int] = None
    for m in MUSEUM_URI_RE.finditer(text):
        found = int(m.group(1))
    return found


def _hit_to_source(hit: dict) -> dict:
    law = (hit.get("law_name") or hit.get("museum") or "").strip() or "未知法规"
    article = (hit.get("article") or "").strip()
    chunk_id = (hit.get("chunk_id") or "").strip()
    url = (hit.get("source") or hit.get("url") or hit.get("detail_url") or "").strip()
    object_id = article or chunk_id or (hit.get("object_id") or "")
    return {
        "museum": law,
        "url": url or (f"legal://{chunk_id}" if chunk_id else ""),
        "object_id": object_id,
        "accession_number": chunk_id or None,
    }


async def _build_sources_from_outputs(outputs: list) -> list[dict]:
    """从工具快照提取 sources（支持 search_legal_docs）。"""
    seen: set[str] = set()
    sources: list[dict] = []
    for item in outputs:
        tool, content, _, _ = _unpack_snapshot(item)
        records = _records_from_tool_output(content)
        for rec in records:
            if tool == "search_legal_docs" or rec.get("law_name") or rec.get("chunk_id"):
                row = _hit_to_source(rec)
            else:
                row = {
                    "museum": rec.get("museum") or rec.get("museum_name") or "未知法规",
                    "url": rec.get("detail_url") or rec.get("url") or "",
                    "object_id": rec.get("object_id") or "",
                }
            _append_source_if_new(seen, sources, row)
            if len(sources) >= MAX_SOURCES:
                return sources
    return sources


async def extract_sources_with_mysql(
    query_snapshots: list = None,
    messages: list = None,
    timeout: float = 8.0,
) -> list[dict]:
    """兼容旧签名；现主要用于 FAISS 工具输出解析。"""
    outputs = list(query_snapshots or [])
    try:
        return await asyncio.wait_for(_build_sources_from_outputs(outputs), timeout=timeout)
    except Exception as e:
        logging.warning("[SourceExtractor] extract failed: %s", e)
        return []


def extract_sources_from_messages(messages: list[Any]) -> list[dict]:
    outputs: list = []
    for m in messages or []:
        if isinstance(m, dict) and m.get("role") == "tool":
            outputs.append(("tool", m.get("content") or "", None, None))
    return asyncio.run(_build_sources_from_outputs(outputs))
