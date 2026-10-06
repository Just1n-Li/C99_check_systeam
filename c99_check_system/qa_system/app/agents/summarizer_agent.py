"""
app/agents/summarizer_agent.py — 总结 Agent

职责：将 Retriever Agent 召回的条文压缩为 RAG context，并提取溯源信息。
"""

from __future__ import annotations

import logging
from typing import Any, Optional

from config import settings


def chunks_to_sources(hits: list[dict]) -> list[dict]:
    """
    将 FAISS hits 转为与现有前端/会话兼容的 sources 结构：
      museum  -> 标准名
      url     -> 来源链接（source_url）
      object_id -> 条款编号或 chunk_id
      accession_number -> chunk_id
    """
    sources: list[dict] = []
    seen: set[str] = set()
    for h in hits:
        law = (h.get("law_name") or "").strip() or "未知条款"
        article = (h.get("article") or "").strip()
        chunk_id = (h.get("chunk_id") or "").strip()
        url = (h.get("source") or h.get("source_url") or "").strip()
        object_id = article or chunk_id
        status = h.get("status")
        key = f"{law}|{object_id}|{url}|{chunk_id}"
        if key in seen:
            continue
        seen.add(key)
        label = law
        if status:
            label = f"{law}（{status}）"
        sources.append({
            "museum": label,
            "url": url or f"legal://{chunk_id or object_id}",
            "object_id": object_id,
            "image_url": None,
            "accession_number": chunk_id or None,
        })
    return sources


def format_hits_as_context(hits: list[dict], max_chars: int = 6000) -> str:
    """确定性格式化检索结果为 RAG context（不依赖 LLM，保证可离线演示）。"""
    if not hits:
        return ""

    parts: list[str] = []
    total = 0
    for i, h in enumerate(hits, 1):
        law = h.get("law_name") or "未知条款"
        article = h.get("article") or ""
        text = (h.get("text") or "").strip()
        score = h.get("score")
        score_s = f"（相关度 {score:.3f}）" if isinstance(score, (int, float)) else ""
        block = f"【条款 {i}】{law} {article}{score_s}\n{text}"
        if total + len(block) > max_chars:
            remain = max_chars - total
            if remain > 80:
                parts.append(block[:remain] + "…")
            break
        parts.append(block)
        total += len(block) + 2
    return "\n\n".join(parts)


async def run_summarizer_agent(
    question: str,
    hits: list[dict],
    use_llm: bool = True,
) -> dict:
    """
    总结检索结果。

    Returns:
      {
        "rag_context": str,
        "sources": list[dict],
        "has_kg_facts": bool,   # 兼容旧字段：表示是否有检索事实
        "kg_facts": str,
      }
    """
    sources = chunks_to_sources(hits)
    base_context = format_hits_as_context(hits)

    if not hits or not base_context.strip():
        logging.info("[SummarizerAgent] No hits to summarize")
        return {
            "rag_context": "",
            "sources": [],
            "has_kg_facts": False,
            "kg_facts": "",
        }

    rag_context = base_context
    if use_llm:
        try:
            rag_context = await _llm_summarize(question, base_context)
            if not rag_context.strip():
                rag_context = base_context
        except Exception as e:
            logging.warning("[SummarizerAgent] LLM summarize failed, use raw context: %s", e)
            rag_context = base_context

    logging.info(
        "[SummarizerAgent] rag_context=%d chars, sources=%d",
        len(rag_context),
        len(sources),
    )
    return {
        "rag_context": rag_context,
        "sources": sources,
        "has_kg_facts": True,
        "kg_facts": base_context,
    }


async def _llm_summarize(question: str, context: str) -> str:
    from openai import AsyncOpenAI

    client = AsyncOpenAI(
        api_key=settings.resolved_llm_api_key(),
        base_url=settings.resolved_llm_base_url(),
        timeout=settings.LLM_TIMEOUT,
    )
    system = (
        "你是 C99 条款总结 Agent。"
        "将检索到的条款整理为简洁、准确的要点，保留条款编号与标准名，"
        "不要编造未出现的条款内容。直接输出整理后的文本。"
    )
    user = f"用户问题：{question}\n\n检索到的条款：\n{context}\n\n请整理为 RAG 上下文："
    resp = await client.chat.completions.create(
        model=settings.LLM_MODEL_NAME,
        messages=[
            {"role": "system", "content": system},
            {"role": "user", "content": user},
        ],
        temperature=0,
        max_tokens=1024,
    )
    return (resp.choices[0].message.content or "").strip()
