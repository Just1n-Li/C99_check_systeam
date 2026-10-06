"""
app/core/qa_engine.py — 问答引擎总调度

职责：串联 RAG 流程，是接口层和各功能模块之间的桥梁。
流程：Retriever Agent → Summarizer Agent → AnswerBuilder
"""

from app.core.answer_builder import AnswerBuilder
from app.retrieval.llm_generator import LLMGenerator


class QAEngine:
    """问答流程的总调度器。"""

    def __init__(self):
        self.llm = LLMGenerator()
        self.answer_builder = AnswerBuilder(llm_generator=self.llm)

    def create_graph_agent(self):
        from app.agents.graph_agent import create_graph_agent as _create
        return _create()

    async def process(self, question: str, session_id: str = None) -> dict:
        """处理一次用户提问，返回完整的 AskResponse。"""
        from app.agents.graph_agent import run_graph_agent

        graph_result = await run_graph_agent(
            question=question,
            session_id=session_id or "",
            chat_history=None,
        )

        rag_context = graph_result.get("rag_context") or ""
        sources_raw = graph_result.get("sources") or []

        response = await self.answer_builder.build(
            question=question,
            intent=graph_result.get("intent_label") or "LEGAL_QA",
            entity="",
            kg_results=[{"text": rag_context, "source": "faiss"}],
            intermediate_steps=None,
        )

        # 用 FAISS 溯源覆盖 AnswerBuilder 默认空 sources
        if sources_raw and hasattr(response, "sources"):
            from app.models.schemas import SourceInfo
            response.sources = [
                SourceInfo(
                    museum_name=s.get("museum", "未知法规"),
                    detail_url=s.get("url", ""),
                    object_id=s.get("object_id", ""),
                    image_url=s.get("image_url"),
                    accession_number=s.get("accession_number"),
                )
                for s in sources_raw
            ]
            response.has_kg_facts = bool(graph_result.get("has_kg_facts"))

        return response
