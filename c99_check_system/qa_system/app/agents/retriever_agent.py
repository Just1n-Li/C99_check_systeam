"""
app/agents/retriever_agent.py — 检索 Agent

职责：
1. 结合多轮对话理解用户问题（指代消解 / 查询改写，可选）
2. 调用 FAISS Tool 检索相关法律法规条文
3. 返回 hits 列表供 Summarizer Agent 使用
"""

from __future__ import annotations

import asyncio
import json
import logging
import re
from typing import Any, Callable, Optional

from config import settings
from app.agents.tools.faiss_tool import search_legal_docs


# C99 条款编号（如 6.5 / 6.5.2.2），命中则走精确匹配而非语义检索
ARTICLE_RE = re.compile(r"^\s*\d+(?:\.\d+)*\s*$")


# 中文关键词 → 条款编号（§6.5 表达式/运算符）。
# 向量检索对短技术词的跨语言匹配很弱（如「整数乘除」会误中按位运算），
# 这里做确定性兜底：命中关键词直接精确定位条款。越具体的词越靠前，
# 命中后从问句中移除该词，避免子串重复命中。
C99_KEYWORD_ARTICLES = [
    ("复合字面量", ["6.5.2.5"]),
    ("函数调用", ["6.5.2.2"]),
    ("结构体成员", ["6.5.2.3"]),
    ("后缀自增", ["6.5.2.4"]),
    ("后缀自减", ["6.5.2.4"]),
    ("前缀自增", ["6.5.3.1"]),
    ("前缀自减", ["6.5.3.1"]),
    ("强制类型转换", ["6.5.4"]),
    ("强制转换", ["6.5.4"]),
    ("类型转换", ["6.5.4"]),
    ("取地址", ["6.5.3.2"]),
    ("解引用", ["6.5.3.2"]),
    ("取模", ["6.5.5"]),
    ("求余", ["6.5.5"]),
    ("乘除", ["6.5.5"]),
    ("乘法", ["6.5.5"]),
    ("除法", ["6.5.5"]),
    ("加减", ["6.5.6"]),
    ("加法", ["6.5.6"]),
    ("减法", ["6.5.6"]),
    ("左移", ["6.5.7"]),
    ("右移", ["6.5.7"]),
    ("位移", ["6.5.7"]),
    ("移位", ["6.5.7"]),
    ("按位异或", ["6.5.11"]),
    ("按位与", ["6.5.10"]),
    ("按位或", ["6.5.12"]),
    ("异或", ["6.5.11"]),
    ("位运算", ["6.5.10", "6.5.11", "6.5.12"]),
    ("逻辑与", ["6.5.13"]),
    ("逻辑或", ["6.5.14"]),
    ("三元", ["6.5.15"]),
    ("三目", ["6.5.15"]),
    ("自增", ["6.5.2.4", "6.5.3.1"]),
    ("自减", ["6.5.2.4", "6.5.3.1"]),
    ("逗号运算符", ["6.5.17"]),
    ("赋值", ["6.5.16"]),
    ("相等", ["6.5.9"]),
    ("比较", ["6.5.8"]),
    ("sizeof", ["6.5.3.4"]),
]


def _keyword_articles(query: str) -> list[str]:
    """按关键词命中条款编号（去重、保持命中顺序）。"""
    q = query
    articles: list[str] = []
    for kw, arts in C99_KEYWORD_ARTICLES:
        if kw in q:
            for a in arts:
                if a not in articles:
                    articles.append(a)
            q = q.replace(kw, " ", 1)
    return articles


RETRIEVER_SYSTEM_PROMPT = """你是 C99 条款检索 Agent。
根据用户问题，输出一句适合向量检索的简短查询（可保留条款关键词、条款编号）。
只输出查询文本本身，不要解释、不要引号、不要换行。
若问题含「它/上述/刚才」等指代，结合对话历史还原完整检索意图。"""


async def _rewrite_query(
    question: str,
    chat_history: Optional[list],
) -> str:
    """可选：用 LLM 改写检索查询；失败则回退原问题。"""
    from app.core.chat_history import prepare_chat_history, format_history_block

    prepared = prepare_chat_history(chat_history, question)
    history_block = format_history_block(prepared)

    # 无历史或问题已较完整时直接用原问
    if not history_block:
        return question.strip()

    try:
        from openai import AsyncOpenAI

        client = AsyncOpenAI(
            api_key=settings.resolved_llm_api_key(),
            base_url=settings.resolved_llm_base_url(),
            timeout=min(settings.LLM_TIMEOUT, 30),
        )
        user_content = (
            f"对话历史：\n{history_block}\n\n当前问题：{question}\n\n请输出检索查询："
        )
        resp = await client.chat.completions.create(
            model=settings.LLM_MODEL_NAME,
            messages=[
                {"role": "system", "content": RETRIEVER_SYSTEM_PROMPT},
                {"role": "user", "content": user_content},
            ],
            temperature=0,
            max_tokens=128,
        )
        rewritten = (resp.choices[0].message.content or "").strip()
        return rewritten or question.strip()
    except Exception as e:
        logging.warning("[RetrieverAgent] query rewrite failed, use raw question: %s", e)
        return question.strip()


async def run_retriever_agent(
    question: str,
    session_id: str = "",
    chat_history: Optional[list] = None,
    top_k: Optional[int] = None,
    on_tool_call: Optional[Callable[..., Any]] = None,
    on_tool_result: Optional[Callable[..., Any]] = None,
    stop_event: Optional[asyncio.Event] = None,
    rewrite_query: bool = True,
) -> dict:
    """
    执行检索。

    Returns:
      {
        "query": str,
        "hits": list[dict],
        "raw_json": str,
        "intent_label": str,
      }
    """
    logging.info("[RetrieverAgent] Processing: %s...", question[:50])

    if stop_event and stop_event.is_set():
        return {"query": question, "hits": [], "raw_json": "{}", "intent_label": "STOPPED"}

    query = question.strip()

    # C99 条款编号精确匹配（如 6.5 / 6.5.2.2）→ 直接查表，跳过语义检索与查询改写
    if ARTICLE_RE.match(query):
        from app.retrieval.faiss_retriever import get_faiss_retriever
        exact_hits = get_faiss_retriever().get_by_article(query)
        logging.info("[RetrieverAgent] article exact match '%s': %d hit(s)", query, len(exact_hits))
        if on_tool_call:
            await on_tool_call("search_legal_docs", json.dumps({"query": query, "exact_article": True}, ensure_ascii=False))
        if on_tool_result:
            await on_tool_result("search_legal_docs", json.dumps({"count": len(exact_hits)}, ensure_ascii=False))
        return {
            "query": query,
            "hits": exact_hits,
            "raw_json": json.dumps({"query": query, "count": len(exact_hits), "hits": exact_hits}, ensure_ascii=False),
            "intent_label": "EXACT_ARTICLE" if exact_hits else "NOT_FOUND",
        }

    # 中文关键词 → 条款兜底：命中则精确匹配对应条款，跳过语义检索
    kw_articles = _keyword_articles(query)
    if kw_articles:
        from app.retrieval.faiss_retriever import get_faiss_retriever
        kw_hits: list[dict] = []
        for a in kw_articles:
            kw_hits.extend(get_faiss_retriever().get_by_article(a))
        logging.info("[RetrieverAgent] keyword match '%s' -> %s", query, kw_articles)
        if on_tool_call:
            await on_tool_call(
                "search_legal_docs",
                json.dumps({"query": query, "keyword": kw_articles}, ensure_ascii=False),
            )
        if on_tool_result:
            await on_tool_result("search_legal_docs", json.dumps({"count": len(kw_hits)}, ensure_ascii=False))
        return {
            "query": query,
            "hits": kw_hits,
            "raw_json": json.dumps({"query": query, "count": len(kw_hits), "hits": kw_hits}, ensure_ascii=False),
            "intent_label": "KEYWORD_MATCH" if kw_hits else "NOT_FOUND",
        }

    if rewrite_query:
        query = await _rewrite_query(question, chat_history)
        logging.info("[RetrieverAgent] Search query: %s", query[:80])

    if on_tool_call:
        await on_tool_call("search_legal_docs", json.dumps({"query": query}, ensure_ascii=False))

    raw = await search_legal_docs(query, top_k=top_k or settings.RAG_TOP_K)

    if on_tool_result:
        await on_tool_result("search_legal_docs", raw)

    hits: list[dict] = []
    try:
        data = json.loads(raw)
        hits = data.get("hits") or []
        if data.get("error"):
            logging.error("[RetrieverAgent] search error: %s", data["error"])
    except (json.JSONDecodeError, TypeError) as e:
        logging.error("[RetrieverAgent] invalid tool output: %s", e)

    logging.info("[RetrieverAgent] Got %d hit(s)", len(hits))
    return {
        "query": query,
        "hits": hits,
        "raw_json": raw,
        "intent_label": "LEGAL_RETRIEVE" if hits else "NOT_FOUND",
    }
