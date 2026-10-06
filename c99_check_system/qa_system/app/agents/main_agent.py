"""
app/agents/main_agent.py — 问答 Agent（QA Agent）

职责：
1. 接收用户问题
2. 调用 Retriever + Summarizer（经 run_graph_agent 兼容入口）获取 RAG context
3. 基于法规上下文流式生成最终回答
4. 支持前端停止信号
"""

import asyncio
import logging
from typing import Callable, Optional, Any

from openai import AsyncOpenAI
from config import settings
from app.agents.graph_agent import run_graph_agent


TOOL_DISPLAY_NAMES = {
    "search_legal_docs": "FAISS 条款检索",
    "execute_sql": "MySQL 数据库",
    "query_neo4j": "Neo4j 知识图谱",
    "summarize_result": "结果整理",
}


def _tool_display_name(tool_name: str) -> str:
    return TOOL_DISPLAY_NAMES.get(tool_name, tool_name)


async def run_main_agent(
    question: str,
    history: list,
    token_callback: Callable[[str], None],
    done_callback: Callable[[Any], None],
    session_id: str = "",
    agent_step_callback: Optional[Callable[..., None]] = None,
    stop_event: Optional[asyncio.Event] = None,
) -> None:
    from app.core.chat_history import (
        prepare_chat_history,
        build_openai_history_messages,
    )

    logging.info("[QAAgent] Processing: %s...", question[:50])
    logging.info("[QAAgent] Session: %s...", session_id[:8] if session_id else "none")

    prepared_history = prepare_chat_history(history, question)
    if prepared_history:
        logging.info("[QAAgent] Chat history: %d prior message(s)", len(prepared_history))

    if stop_event is None:
        stop_event = asyncio.Event()

    async def emit_step(step_type: str, content: str, tool_name: str = ""):
        if not agent_step_callback:
            return
        try:
            await agent_step_callback(step_type, content, tool_name=tool_name)
        except TypeError:
            await agent_step_callback(step_type, content)

    async def on_tool_call(tool_name: str, tool_args: str):
        logging.info("[QAAgent] Tool called: %s", tool_name)
        if agent_step_callback:
            display = _tool_display_name(tool_name)
            await emit_step("tool_call", f"正在查询 {display}", tool_name=tool_name)

    async def on_tool_result(tool_name: str, result: str):
        logging.info("[QAAgent] Tool result: %s, len=%d", tool_name, len(result))
        if agent_step_callback:
            display = _tool_display_name(tool_name)
            if "error" in result[:80].lower():
                await emit_step("tool_result", f"{display} 执行失败", tool_name=tool_name)
            else:
                try:
                    import json as json_mod
                    data = json_mod.loads(result)
                    count = data.get("count")
                    if count is not None:
                        await emit_step(
                            "tool_result",
                            f"{display} 返回 {count} 条结果",
                            tool_name=tool_name,
                        )
                    else:
                        await emit_step("tool_result", f"{display} 已返回", tool_name=tool_name)
                except Exception:
                    await emit_step("tool_result", f"{display} 已返回", tool_name=tool_name)

    if agent_step_callback:
        await emit_step("thinking", "检索相关 C99 条款…")

    logging.info("[QAAgent] Calling Retriever + Summarizer…")
    graph_result = await run_graph_agent(
        question=question,
        session_id=session_id,
        chat_history=history,
        stop_event=stop_event,
        on_tool_call=on_tool_call,
        on_tool_result=on_tool_result,
    )

    logging.info(
        "[QAAgent] RAG context: %d chars, sources: %d",
        len(graph_result.get("rag_context", "")),
        len(graph_result.get("sources", [])),
    )

    if stop_event.is_set():
        logging.info("[QAAgent] Stopped after retrieval")
        await done_callback(None)
        return

    rag_context = graph_result.get("rag_context", "")
    sources = graph_result.get("sources", [])
    has_kg_facts = bool(graph_result.get("has_kg_facts"))

    if not rag_context:
        logging.warning("[QAAgent] No rag_context from retrieval")
        not_found_msg = (
            "抱歉，在 C99 标准知识库中未检索到与该问题直接相关的条款。"
            "您可以换一种问法，或补充条款编号、关键词后重试。"
        )
        await token_callback(not_found_msg)
        await done_callback({
            "content": not_found_msg,
            "has_kg_facts": False,
            "has_llm_content": False,
            "intent_label": "NOT_FOUND",
            "sources": sources,
        })
        return

    if agent_step_callback:
        await emit_step("thinking", "根据检索结果生成回答…")

    client = AsyncOpenAI(
        api_key=settings.resolved_llm_api_key(),
        base_url=settings.resolved_llm_base_url(),
        timeout=settings.LLM_TIMEOUT,
    )

    system_prompt = """你是一个 C 语言（C99 标准）编译器符合性测试代码生成器。

根据给定的 C99 条款原文，生成一个 C 测试程序，**同时包含「正向测试」和「负向测试」两部分**：

【正向测试】验证条款的语义（Semantics）：能编译、能运行的正常 C 代码，用 assert 或 printf 验证运行结果，覆盖条款的每条语义要求。

【负向测试】验证条款的约束（Constraints）：写出故意违反约束的代码片段，期望编译器拒绝（编译报错）。

格式规则：
1. 完整代码放在一个 ```c 代码块内。
2. 用醒目的注释把两部分分开，让读者一眼看出哪里是正向、哪里是负向，例如：
   /* ========== 正向测试：以下代码应能编译并运行通过 ========== */
   （正常 C 代码 + assert）
   /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
   #if 0
   /* 违反约束「操作数必须为算术类型」：结构体不能相乘，gcc -std=c99 应报错 */
   struct S { int x; } a, b;
   a * b;
   #endif
3. 所有负向片段统一放在同一个 `#if 0 ... #endif` 块里，保证整个文件仍能正常编译运行；每个负向片段前用注释说明违反了哪条约束、期望什么报错。
4. 负向测试只针对「约束（constraint）」（违反后编译器必须报错的规则）；不要把「未定义行为(UB)」当作负向测试——UB 代码仍能编译通过，只是运行行为未定义。
5. 程序开头注释写明：验证 C99 哪一条款、预期行为（正向运行通过、负向编译报错）。
6. 严格基于条款内容，不要引入条款未涉及的特性。
7. 代码块之外可附 1~3 句中文说明，但代码块是主体。
8. 逐条覆盖条款的每个编号段落（[1][2]…）、Constraints、Semantics、EXAMPLE，不要漏任何一段；每个测试用注释标注对应段落号（如 /* [6] 无原型默认实参提升 */）。
9. 这些语义最容易被漏，务必测到：无原型函数调用的默认实参提升（char→int、float→double）与省略号（...）后停止转换；「非左值」结果（函数返回结构体的成员 f().x、强制转换、条件/逗号表达式的结果）用负向测试验证「对它赋值应编译报错」；const/volatile 限定类型在成员访问与解引用上的传播。"""

    current_user_content = f"""C99 条款：{question}

条款原文（C99 标准）：
{rag_context}

请根据以上条款，生成用于测试编译器是否符合该条款的 C 测试程序："""

    messages: list[dict[str, str]] = [{"role": "system", "content": system_prompt}]
    messages.extend(build_openai_history_messages(prepared_history))
    messages.append({"role": "user", "content": current_user_content})

    logging.info("[QAAgent] Calling LLM for final answer…")

    # 推理模型（sensenova-*）先流式输出 reasoning_content，再输出 content；
    # reasoning 长度不确定，偶发把 max_tokens 占满导致 content 为空。
    # 对「空内容」做有限次重试（reasoning 不会复现，重试大概率命中正文）。
    MAX_EMPTY_CONTENT_RETRIES = 2

    async def _stream_once() -> str:
        stream = await client.chat.completions.create(
            model=settings.LLM_MODEL_NAME,
            messages=messages,
            stream=True,
            temperature=settings.LLM_TEMPERATURE,
            max_tokens=settings.LLM_MAX_TOKENS,
        )
        full = ""
        async for chunk in stream:
            if stop_event.is_set():
                break
            if not chunk.choices:
                continue
            delta = chunk.choices[0].delta
            if delta is None:
                continue
            if delta.content:
                full += delta.content
                await token_callback(delta.content)
        return full

    try:
        full_response = ""
        for attempt in range(MAX_EMPTY_CONTENT_RETRIES + 1):
            logging.info("[QAAgent] LLM stream attempt %d started…", attempt + 1)
            full_response = await _stream_once()

            if stop_event.is_set():
                logging.info("[QAAgent] Stopped during streaming")
                if full_response:
                    await done_callback({
                        "content": full_response,
                        "has_kg_facts": has_kg_facts,
                        "has_llm_content": True,
                        "intent_label": graph_result.get("intent_label", "LEGAL_QA"),
                        "sources": sources,
                    })
                else:
                    await done_callback(None)
                return

            if full_response.strip():
                break
            logging.warning(
                "[QAAgent] LLM returned empty content (reasoning overflow?), "
                "retrying %d/%d…", attempt + 1, MAX_EMPTY_CONTENT_RETRIES,
            )

        logging.info("[QAAgent] Streaming finished, total: %d chars", len(full_response))
        await done_callback({
            "content": full_response,
            "has_kg_facts": has_kg_facts,
            "has_llm_content": bool(full_response.strip()),
            "intent_label": graph_result.get("intent_label", "LEGAL_QA"),
            "sources": sources,
        })

    except Exception as e:
        logging.error("[QAAgent] LLM call failed: %s", e)
        await done_callback({
            "content": f"生成回答时出错: {str(e)}",
            "has_kg_facts": has_kg_facts,
            "has_llm_content": True,
            "intent_label": "ERROR",
            "sources": sources,
        })
