"""
app/agents/tools/faiss_tool.py — FAISS 法规检索 Tool

供 Retriever Agent 调用。
"""

from __future__ import annotations

import json
from typing import Optional

from config import settings


async def search_legal_docs(query: str, top_k: Optional[int] = None) -> str:
    """
    在 C99 标准 FAISS 向量库中语义检索相关条款。

    Args:
        query: 检索查询（用户问题或改写后的查询）
        top_k: 返回条数，默认取配置 RAG_TOP_K

    Returns:
        JSON 字符串，含 hits 列表（chunk_id / law_name / article / text / source / score）
    """
    from app.retrieval.faiss_retriever import get_faiss_retriever

    retriever = get_faiss_retriever()
    k = top_k if top_k is not None else settings.RAG_TOP_K
    try:
        hits = retriever.search(query, top_k=k)
    except Exception as e:
        return json.dumps({"error": str(e), "hits": []}, ensure_ascii=False)

    return json.dumps(
        {
            "query": query,
            "count": len(hits),
            "hits": hits,
        },
        ensure_ascii=False,
    )


# LangChain @tool 可选包装（create_agent 工厂用）
try:
    from langchain_core.tools import tool

    @tool
    async def search_legal_docs_tool(query: str, top_k: int = 5) -> str:
        """在 C99 标准向量库中检索相关条款。query 为问题关键词，top_k 为返回条数。"""
        return await search_legal_docs(query, top_k=top_k)

except ImportError:
    search_legal_docs_tool = None
