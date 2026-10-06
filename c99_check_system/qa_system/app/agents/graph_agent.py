"""
app/agents/graph_agent.py — 兼容层

历史名称「Graph Agent」现指向法律法规「检索 + 总结」流水线：
  Retriever Agent (FAISS) → Summarizer Agent

对外仍暴露 run_graph_agent / create_graph_agent，避免 session_manager / qa_engine 大改。
"""

from __future__ import annotations

import asyncio
import logging
from typing import Callable, Optional

from config import settings


# 遗留常量：保持 import 不报错（main_qa_agent 等）
MYSQL_TOOLS: list = []
NEO4J_TOOLS: list = []
NEO4J_TOOL_NAMES: set = set()


def get_available_tools() -> list:
    """返回 FAISS 检索相关的 OpenAI tools 定义。"""
    return [
        {
            "type": "function",
            "function": {
                "name": "search_legal_docs",
                "description": "在 C99 标准 FAISS 向量库中语义检索相关条款。",
                "parameters": {
                    "type": "object",
                    "properties": {
                        "query": {
                            "type": "string",
                            "description": "检索查询文本",
                        },
                        "top_k": {
                            "type": "integer",
                            "description": "返回条数，默认 5",
                        },
                    },
                    "required": ["query"],
                },
            },
        }
    ]


def create_graph_agent(use_streaming: bool = False):
    """兼容旧工厂 API；实际执行走 run_graph_agent。"""
    try:
        from langchain.agents import create_agent as _lc_create_agent
        from app.retrieval.llm_generator import create_llm
        from app.agents.tools.faiss_tool import search_legal_docs_tool

        tools = [search_legal_docs_tool] if search_legal_docs_tool else []
        llm = create_llm(temperature=0)
        return _lc_create_agent(
            model=llm,
            tools=tools,
            system_prompt="你是 C99 条款检索 Agent，使用 search_legal_docs 检索相关条款。",
        )
    except Exception:
        return None


async def run_graph_agent(
    question: str,
    session_id: str = "",
    chat_history: Optional[list] = None,
    on_thinking: Optional[Callable[[str], None]] = None,
    on_tool_call: Optional[Callable[[str, str], None]] = None,
    on_tool_result: Optional[Callable[[str, str], None]] = None,
    stop_event: Optional[asyncio.Event] = None,
) -> dict:
    """
    检索 Agent → 总结 Agent。

    返回结构与旧版兼容：
      rag_context, kg_facts, has_kg_facts, intent_label, sources
    """
    from app.agents.retriever_agent import run_retriever_agent
    from app.agents.summarizer_agent import run_summarizer_agent

    logging.info("[GraphAgent/Compat] → Retriever + Summarizer pipeline")

    if stop_event is None:
        stop_event = asyncio.Event()

    retrieve_result = await run_retriever_agent(
        question=question,
        session_id=session_id,
        chat_history=chat_history,
        top_k=settings.RAG_TOP_K,
        on_tool_call=on_tool_call,
        on_tool_result=on_tool_result,
        stop_event=stop_event,
    )

    if stop_event.is_set():
        return {
            "rag_context": "",
            "kg_facts": "",
            "has_kg_facts": False,
            "intent_label": "STOPPED",
            "sources": [],
        }

    hits = retrieve_result.get("hits") or []
    summary = await run_summarizer_agent(
        question=question,
        hits=hits,
        use_llm=False,  # 测试代码生成需完整条款原文，禁用 LLM 压缩总结
    )

    return {
        "rag_context": summary.get("rag_context", ""),
        "kg_facts": summary.get("kg_facts", ""),
        "has_kg_facts": summary.get("has_kg_facts", False),
        "intent_label": retrieve_result.get("intent_label", "LEGAL_RETRIEVE"),
        "sources": summary.get("sources", []),
    }
