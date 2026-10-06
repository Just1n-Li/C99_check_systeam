"""
Main QA Agent — 遗留非流式入口（已优先走 SessionManager + main_agent）

保留 generate_polished_answer 等辅助方法；持久化一律走 SessionManager（SQLite/MySQL）。
"""

from typing import Optional
from app.retrieval.llm_generator import create_llm


class MainQAAgent:
    def __init__(self):
        self._llm = None

    def _get_llm(self):
        if self._llm is None:
            self._llm = create_llm(temperature=0.1)
        return self._llm

    async def process(
        self,
        question: str,
        session_id: str,
        chat_history: list[dict],
    ) -> dict:
        """兼容旧调用：检索+总结，并由 SessionManager 负责落库。"""
        from app.agents.graph_agent import run_graph_agent
        from app.core.session_manager import get_session_manager

        graph_result = await run_graph_agent(
            question=question,
            session_id=session_id,
            chat_history=chat_history,
        )

        answer_text = graph_result.get("rag_context") or "暂无相关法规条文。"
        sources = graph_result.get("sources") or []
        intent_label = graph_result.get("intent_label") or "LEGAL_QA"

        sm = get_session_manager()
        user_msg_id = await sm.save_message(session_id, "user", question)
        assistant_msg_id = await sm.save_message(
            session_id, "assistant", answer_text, intent=intent_label
        )
        await sm.update_stream_done(
            session_id,
            assistant_msg_id,
            sources=sources,
            entity={
                "has_kg_facts": bool(graph_result.get("has_kg_facts")),
                "has_llm_content": False,
            },
        )

        return {
            "answer_text": answer_text,
            "sources": sources,
            "intent_label": intent_label,
            "user_msg_id": user_msg_id,
            "assistant_msg_id": assistant_msg_id,
        }

    async def generate_polished_answer(
        self,
        question: str,
        answer_text: str,
        sources: Optional[list] = None,
    ) -> str:
        if not answer_text or answer_text.startswith("暂无"):
            return answer_text
        llm = self._get_llm()
        prompt = (
            f"用户问题：{question}\n\n法条检索结果：\n{answer_text}\n\n"
            "请基于以上法条给出简洁准确的中文回答，引用法律名称与条文编号，不要编造。"
        )
        try:
            result = await llm.ainvoke(prompt)
            return getattr(result, "content", None) or str(result)
        except Exception as e:
            return f"{answer_text}\n\n（润色失败：{e}）"


_main_qa_agent: Optional[MainQAAgent] = None


def get_main_qa_agent() -> MainQAAgent:
    global _main_qa_agent
    if _main_qa_agent is None:
        _main_qa_agent = MainQAAgent()
    return _main_qa_agent
