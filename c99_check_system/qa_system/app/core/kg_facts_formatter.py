"""
从 Graph Agent 的原始查询结果中提取并格式化「知识图谱事实」文本。

课设要求：LLM 生成的补充性描述须与知识图谱事实性内容明确区分；
本模块只处理 MySQL / Neo4j 工具返回的结构化数据，不包含 summarize_result 等 LLM 总结。
"""

from __future__ import annotations

import json
from typing import Optional

from app.core.source_extractor import QuerySnapshot, _records_from_tool_output, _unpack_snapshot

MAX_FACT_RECORDS = 8

_FIELD_LABELS: tuple[tuple[str, str], ...] = (
    ("title", "名称"),
    ("name", "名称"),
    ("a.title", "名称"),
    ("a.name", "名称"),
    ("museum", "博物馆"),
    ("museum_name", "博物馆"),
    ("m.name", "博物馆"),
    ("dynasty", "朝代"),
    ("d.name", "朝代"),
    ("artist", "作者"),
    ("material", "材质"),
    ("type", "类型"),
    ("artifact_type", "类型"),
    ("dimensions", "尺寸"),
    ("period", "时期"),
    ("culture", "文化"),
    ("location", "所在地"),
    ("accession_number", "馆藏编号"),
    ("object_id", "编号"),
    ("description", "描述"),
)


def _pick_title(record: dict) -> str:
    for key in ("title", "name", "a.title", "a.name"):
        value = record.get(key)
        if value and str(value).strip():
            return str(value).strip()
    object_id = record.get("object_id") or record.get("a.object_id")
    if object_id:
        return str(object_id)
    return "未知文物"


def _format_record_lines(record: dict) -> list[str]:
    seen_labels: set[str] = set()
    lines: list[str] = []

    for key, label in _FIELD_LABELS:
        if label in seen_labels:
            continue
        raw = record.get(key)
        if raw is None:
            continue
        text = str(raw).strip()
        if not text or text.lower() in {"null", "none"}:
            continue
        if label == "名称":
            seen_labels.add(label)
            continue
        seen_labels.add(label)
        if len(text) > 280:
            text = text[:280] + "…"
        lines.append(f"- **{label}**：{text}")

    return lines


def _format_record(record: dict, index: int) -> Optional[str]:
    title = _pick_title(record)
    body_lines = _format_record_lines(record)
    if not body_lines and title == "未知文物":
        return None

    header = f"**{index}. {title}**"
    if not body_lines:
        return header
    return header + "\n" + "\n".join(body_lines)


def format_kg_facts_from_snapshots(
    query_snapshots: list[QuerySnapshot] | None,
) -> tuple[str, bool]:
    """
    将 execute_sql / query_neo4j 的原始返回格式化为可读事实文本。

    Returns:
        (kg_facts_text, has_kg_facts)
    """
    if not query_snapshots:
        return "", False

    blocks: list[str] = []
    seen_keys: set[str] = set()

    for item in query_snapshots:
        tool_name, content, _museum_hint, _query_text = _unpack_snapshot(item)
        if tool_name not in {"execute_sql", "query_neo4j"}:
            continue
        for record in _records_from_tool_output(content):
            dedupe_key = "|".join(
                str(record.get(k) or "")
                for k in ("object_id", "uri", "title", "name", "a.uri", "a.name")
            ).strip()
            if dedupe_key and dedupe_key in seen_keys:
                continue
            if dedupe_key:
                seen_keys.add(dedupe_key)

            block = _format_record(record, len(blocks) + 1)
            if block:
                blocks.append(block)
            if len(blocks) >= MAX_FACT_RECORDS:
                break
        if len(blocks) >= MAX_FACT_RECORDS:
            break

    if not blocks:
        return "", False

    intro = (
        "以下内容由 **MySQL 结构化库** 与 **Neo4j 知识图谱** 查询直接返回，"
        "为可溯源的事实性数据（非大语言模型生成）。"
    )
    return intro + "\n\n" + "\n\n".join(blocks), True
