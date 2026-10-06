"""
app/core/query_memory.py — 短时查询记忆

为 RAG Agent 提供跨轮次的数据库查询上下文记忆。

设计：
- session_id → List[QueryRecord] 的内存映射
- 每次 RAG 调用 MySQL execute_sql / Neo4j query_neo4j / Neo4j get_graph_schema 等工具时
  记录：tool_name + query/cypher + results
- 下次 RAG 调用时，将历史附加到 context 中，让 LLM 看到之前问过什么
- 内存存储，不持久化

支持的 tool_name：
- "execute_sql"（MySQL）
- "query_neo4j" / "get_graph_schema" / "explore_graph_sample" / "count_nodes_by_label"（Neo4j）
"""

import logging
import time
from dataclasses import dataclass
from typing import Optional


@dataclass
class QueryRecord:
    """一次工具调用的执行记录"""
    tool_name: str          # 工具名，例如 "execute_sql" / "query_neo4j"
    query: str              # SQL 语句 或 Cypher 语句；其他工具的描述
    results: str            # JSON 字符串（结果）
    result_count: int = 0   # 返回条数
    timestamp: int = 0

    def get_summary(self) -> str:
        """生成简洁的摘要，注入到下一轮 LLM 的 system prompt。"""
        if not self.results:
            return f"[{self.tool_name}] 查询无结果: {self._short_query()}"
        if self.result_count == 0:
            return f"[{self.tool_name}] 查询返回 0 条结果: {self._short_query()}"
        return f"[{self.tool_name}] 查询返回 {self.result_count} 条结果: {self._short_query()}"

    def _short_query(self, max_len: int = 120) -> str:
        s = (self.query or "").replace("\n", " ").strip()
        return s if len(s) <= max_len else s[:max_len] + "..."


class QueryMemory:
    """
    短时查询记忆管理器（MySQL + Neo4j 共用）

    使用示例：
        memory = QueryMemory()
        memory.add_record("session-1", "execute_sql",
                          "SELECT * FROM artifact WHERE museum = 'Harvard' LIMIT 10",
                          "[{...}]", 10)
        memory.add_record("session-1", "query_neo4j",
                          "MATCH (a:Artifact)-[:belongsToDynasty]->(:Dynasty {name:'Tang'}) RETURN count(a)",
                          "[42]", 1)

        ctx = memory.get_full_context("session-1", max_records=10)
    """

    def __init__(self, max_records_per_session: int = 20):
        self._store: dict[str, list[QueryRecord]] = {}
        self._max_records = max_records_per_session

    def add_record(
        self,
        session_id: str,
        tool_name: str,
        query: str,
        results: str,
        result_count: int = 0,
    ) -> None:
        """添加一条工具调用记录。

        Args:
            session_id: 会话 ID
            tool_name: 工具名（"execute_sql" / "query_neo4j" / "get_graph_schema" / ...）
            query: SQL 或 Cypher 语句
            results: 结果 JSON 字符串
            result_count: 结果条数
        """
        if not session_id:
            return
        if session_id not in self._store:
            self._store[session_id] = []

        record = QueryRecord(
            tool_name=tool_name or "unknown",
            query=query or "",
            results=results or "",
            result_count=result_count or 0,
            timestamp=int(time.time()),
        )

        self._store[session_id].append(record)

        # 限制每个 session 的记录数量（FIFO 截断）
        if len(self._store[session_id]) > self._max_records:
            self._store[session_id] = self._store[session_id][-self._max_records:]

        logging.info(
            f"[QueryMemory] Added record for session {session_id[:8]}... "
            f"tool={tool_name} count={result_count} total={len(self._store[session_id])}"
        )

    def get_history(self, session_id: str, max_records: int = 10) -> list[str]:
        """
        获取最近 N 条记录的摘要（按时间正序）。

        Returns:
            历史摘要列表，例如：
            [
              "[execute_sql] 查询返回 5 条结果: SELECT object_id, title FROM artifact ...",
              "[query_neo4j] 查询返回 12 条结果: MATCH (a:Artifact)-[:belongsToDynasty]->(d:Dynasty) ...",
            ]
        """
        if session_id not in self._store:
            return []

        records = self._store[session_id][-max_records:]
        return [r.get_summary() for r in records]

    def get_full_context(self, session_id: str, max_records: int = 10) -> str:
        """
        获取完整的查询上下文字符串，用于附加到 RAG 消息的 system prompt 末尾。
        """
        history = self.get_history(session_id, max_records)
        if not history:
            return ""

        context_lines = ["## 之前的数据库查询记录（供参考，避免重复查）"]
        for i, line in enumerate(history, 1):
            context_lines.append(f"{i}. {line}")
        return "\n".join(context_lines)

    def get_recent_records(self, session_id: str, max_records: int = 10) -> list[QueryRecord]:
        """返回最近 N 条原始记录（含 tool_name），便于调用方做更细的判断。"""
        if session_id not in self._store:
            return []
        return list(self._store[session_id][-max_records:])

    def clear_session(self, session_id: str) -> None:
        """清除指定会话的所有查询记忆。"""
        if session_id in self._store:
            del self._store[session_id]
            logging.info(f"[QueryMemory] Cleared memory for session {session_id[:8]}...")

    def get_record_count(self, session_id: str) -> int:
        """获取指定会话的记录数。"""
        return len(self._store.get(session_id, []))

    def stats(self) -> dict:
        """统计全局状态。"""
        total = sum(len(v) for v in self._store.values())
        return {
            "sessions": len(self._store),
            "total_records": total,
        }


# 全局单例
_query_memory: Optional[QueryMemory] = None


def get_query_memory() -> QueryMemory:
    """获取全局 QueryMemory 实例。"""
    global _query_memory
    if _query_memory is None:
        _query_memory = QueryMemory()
    return _query_memory
