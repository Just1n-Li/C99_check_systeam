"""
app/db/sqlite_session_store.py — 本地 SQLite 会话存储

用于多轮对话历史 / 流式续写 / 反馈，不依赖远程 MySQL。
法规知识库仍在 FAISS，与本模块无关。
"""

from __future__ import annotations

import json
import logging
import sqlite3
import threading
from datetime import datetime, timedelta
from pathlib import Path
from typing import Any, Optional

from config import BASE_DIR, settings

logger = logging.getLogger(__name__)


def _db_path() -> Path:
    raw = getattr(settings, "SQLITE_SESSION_PATH", None) or str(BASE_DIR / "data" / "sessions.db")
    path = Path(raw)
    if not path.is_absolute():
        path = BASE_DIR / path
    path.parent.mkdir(parents=True, exist_ok=True)
    return path


class SqliteSessionStore:
    _instance: Optional["SqliteSessionStore"] = None
    _lock = threading.Lock()

    def __init__(self, db_path: Optional[Path] = None):
        self.db_path = Path(db_path) if db_path else _db_path()
        self._local = threading.local()
        self._schema_ready = False

    @classmethod
    def get_instance(cls) -> "SqliteSessionStore":
        if cls._instance is None:
            with cls._lock:
                if cls._instance is None:
                    cls._instance = SqliteSessionStore()
        return cls._instance

    def _conn(self) -> sqlite3.Connection:
        conn = getattr(self._local, "conn", None)
        if conn is None:
            conn = sqlite3.connect(str(self.db_path), check_same_thread=False)
            conn.row_factory = sqlite3.Row
            conn.execute("PRAGMA journal_mode=WAL")
            conn.execute("PRAGMA foreign_keys=ON")
            self._local.conn = conn
        return conn

    def ensure_schema(self) -> None:
        if self._schema_ready:
            return
        with self._lock:
            if self._schema_ready:
                return
            conn = self._conn()
            conn.executescript(
                """
                CREATE TABLE IF NOT EXISTS ai_history (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    session_id TEXT NOT NULL,
                    role TEXT NOT NULL,
                    content TEXT,
                    tool_name TEXT,
                    tool_input TEXT,
                    tool_output TEXT,
                    intent TEXT,
                    entity TEXT,
                    sources TEXT,
                    token_count INTEGER DEFAULT 0,
                    sent_offset INTEGER DEFAULT 0,
                    streaming_done INTEGER DEFAULT 0,
                    created_at TEXT DEFAULT (datetime('now','localtime'))
                );
                CREATE INDEX IF NOT EXISTS idx_ai_history_session
                    ON ai_history(session_id);
                CREATE INDEX IF NOT EXISTS idx_ai_history_created
                    ON ai_history(created_at);

                CREATE TABLE IF NOT EXISTS ai_feedback (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    message_id INTEGER NOT NULL UNIQUE,
                    session_id TEXT NOT NULL,
                    is_helpful INTEGER NOT NULL,
                    intent TEXT,
                    question TEXT,
                    created_at TEXT DEFAULT (datetime('now','localtime'))
                );
                CREATE INDEX IF NOT EXISTS idx_ai_feedback_session
                    ON ai_feedback(session_id);
                """
            )
            conn.commit()
            self._schema_ready = True
            logger.info("[SqliteSession] schema ready at %s", self.db_path)

    def delete_session(self, session_id: str) -> int:
        self.ensure_schema()
        conn = self._conn()
        conn.execute("DELETE FROM ai_feedback WHERE session_id = ?", (session_id,))
        cur = conn.execute("DELETE FROM ai_history WHERE session_id = ?", (session_id,))
        conn.commit()
        return cur.rowcount

    def save_message(
        self,
        session_id: str,
        role: str,
        content: str,
        tool_name: Optional[str] = None,
        tool_input: Optional[dict] = None,
        tool_output: Optional[str] = None,
        intent: Optional[str] = None,
        entity: Optional[Any] = None,
    ) -> int:
        self.ensure_schema()
        conn = self._conn()
        entity_s = (
            json.dumps(entity, ensure_ascii=False)
            if isinstance(entity, (dict, list))
            else entity
        )
        cur = conn.execute(
            """INSERT INTO ai_history
               (session_id, role, content, tool_name, tool_input, tool_output,
                intent, entity, token_count, sent_offset, streaming_done)
               VALUES (?, ?, ?, ?, ?, ?, ?, ?, 0, 0, 0)""",
            (
                session_id,
                role,
                content,
                tool_name,
                json.dumps(tool_input, ensure_ascii=False) if tool_input else None,
                tool_output,
                intent,
                entity_s,
            ),
        )
        conn.commit()
        return int(cur.lastrowid)

    def append_content(self, session_id: str, message_id: int, new_chunk: str) -> None:
        self.ensure_schema()
        conn = self._conn()
        conn.execute(
            """UPDATE ai_history
               SET content = COALESCE(content, '') || ?,
                   sent_offset = length(COALESCE(content, '') || ?)
               WHERE id = ? AND session_id = ?""",
            (new_chunk, new_chunk, message_id, session_id),
        )
        conn.commit()

    def update_stream_done(
        self,
        session_id: str,
        message_id: int,
        total_tokens: int = 0,
        sources: Optional[list] = None,
        entity: Optional[dict] = None,
    ) -> None:
        self.ensure_schema()
        conn = self._conn()
        sources_json = json.dumps(sources, ensure_ascii=False) if sources else None
        entity_json = json.dumps(entity, ensure_ascii=False) if entity else None
        conn.execute(
            """UPDATE ai_history
               SET streaming_done = 1,
                   token_count = ?,
                   sent_offset = length(COALESCE(content, '')),
                   sources = ?,
                   entity = ?
               WHERE id = ? AND session_id = ?""",
            (total_tokens, sources_json, entity_json, message_id, session_id),
        )
        conn.commit()

    def get_last_message(self, session_id: str) -> Optional[dict]:
        self.ensure_schema()
        conn = self._conn()
        row = conn.execute(
            """SELECT id, role, content, sent_offset, streaming_done
               FROM ai_history
               WHERE session_id = ?
               ORDER BY datetime(created_at) DESC, id DESC
               LIMIT 1""",
            (session_id,),
        ).fetchone()
        return dict(row) if row else None

    def get_message_by_id(self, session_id: str, message_id: int) -> Optional[dict]:
        self.ensure_schema()
        conn = self._conn()
        row = conn.execute(
            """SELECT id, role, content, sent_offset, streaming_done
               FROM ai_history
               WHERE id = ? AND session_id = ?""",
            (message_id, session_id),
        ).fetchone()
        return dict(row) if row else None

    def get_feedback_map(self, session_id: str) -> dict[int, bool]:
        self.ensure_schema()
        conn = self._conn()
        rows = conn.execute(
            """SELECT message_id, is_helpful FROM ai_feedback WHERE session_id = ?""",
            (session_id,),
        ).fetchall()
        return {int(r["message_id"]): bool(r["is_helpful"]) for r in rows}

    def save_feedback(
        self,
        message_id: int,
        is_helpful: bool,
        session_id: Optional[str] = None,
    ) -> dict:
        self.ensure_schema()
        conn = self._conn()
        row = conn.execute(
            """SELECT id, session_id, role, intent FROM ai_history WHERE id = ?""",
            (message_id,),
        ).fetchone()
        if not row:
            raise ValueError("消息不存在")
        if row["role"] != "assistant":
            raise ValueError("仅可对助手回答提交反馈")
        msg_session_id = row["session_id"]
        if session_id and session_id != msg_session_id:
            raise ValueError("会话不匹配")
        intent = row["intent"]

        qrow = conn.execute(
            """SELECT content FROM ai_history
               WHERE session_id = ? AND role = 'user' AND id < ?
               ORDER BY id DESC LIMIT 1""",
            (msg_session_id, message_id),
        ).fetchone()
        question = qrow["content"] if qrow else None

        conn.execute(
            """INSERT INTO ai_feedback (message_id, session_id, is_helpful, intent, question)
               VALUES (?, ?, ?, ?, ?)
               ON CONFLICT(message_id) DO UPDATE SET
                   is_helpful = excluded.is_helpful,
                   intent = excluded.intent,
                   question = excluded.question""",
            (message_id, msg_session_id, 1 if is_helpful else 0, intent, question),
        )
        conn.commit()
        return {
            "message_id": message_id,
            "is_helpful": is_helpful,
            "feedback": "helpful" if is_helpful else "inaccurate",
        }

    def get_history_rows(self, session_id: str, limit: int) -> list[dict]:
        self.ensure_schema()
        conn = self._conn()
        rows = conn.execute(
            """SELECT id, role, content, intent, entity, streaming_done, sources, created_at
               FROM ai_history
               WHERE session_id = ? AND role IN ('user', 'assistant')
               ORDER BY datetime(created_at) ASC, id ASC
               LIMIT ?""",
            (session_id, limit),
        ).fetchall()
        return [dict(r) for r in rows]

    def get_feedback_stats(self, days: int = 30, top_n: int = 20) -> dict:
        self.ensure_schema()
        conn = self._conn()
        since = (datetime.now() - timedelta(days=days)).strftime("%Y-%m-%d %H:%M:%S")

        row = conn.execute(
            """SELECT
                   COUNT(*) AS total,
                   SUM(CASE WHEN is_helpful = 1 THEN 1 ELSE 0 END) AS helpful,
                   SUM(CASE WHEN is_helpful = 0 THEN 1 ELSE 0 END) AS inaccurate
               FROM ai_feedback
               WHERE created_at >= ?""",
            (since,),
        ).fetchone()
        total = int(row["total"] or 0)
        helpful = int(row["helpful"] or 0)
        inaccurate = int(row["inaccurate"] or 0)
        rate = round(inaccurate / total, 4) if total else 0.0

        intent_rows = conn.execute(
            """SELECT
                   COALESCE(intent, 'UNKNOWN') AS intent,
                   SUM(CASE WHEN is_helpful = 0 THEN 1 ELSE 0 END) AS inaccurate_count,
                   COUNT(*) AS total_count
               FROM ai_feedback
               WHERE created_at >= ?
               GROUP BY COALESCE(intent, 'UNKNOWN')
               HAVING inaccurate_count > 0
               ORDER BY inaccurate_count DESC, total_count DESC""",
            (since,),
        ).fetchall()
        by_intent = [
            {
                "intent": r["intent"],
                "inaccurate_count": int(r["inaccurate_count"]),
                "total_count": int(r["total_count"]),
            }
            for r in intent_rows
        ]

        question_rows = conn.execute(
            """SELECT
                   question,
                   COALESCE(intent, 'UNKNOWN') AS intent,
                   COUNT(*) AS cnt,
                   MAX(created_at) AS latest_at
               FROM ai_feedback
               WHERE is_helpful = 0
                 AND question IS NOT NULL
                 AND trim(question) != ''
                 AND created_at >= ?
               GROUP BY question, COALESCE(intent, 'UNKNOWN')
               ORDER BY cnt DESC, latest_at DESC
               LIMIT ?""",
            (since, top_n),
        ).fetchall()
        top_questions = [
            {
                "question": r["question"],
                "intent": r["intent"],
                "count": int(r["cnt"]),
                "latest_at": r["latest_at"],
            }
            for r in question_rows
        ]

        return {
            "days": days,
            "summary": {
                "total_feedback": total,
                "helpful_count": helpful,
                "inaccurate_count": inaccurate,
                "inaccurate_rate": rate,
            },
            "by_intent": by_intent,
            "top_inaccurate_questions": top_questions,
        }

    def get_inaccurate_feedback_list(self, limit: int = 50, offset: int = 0) -> dict:
        self.ensure_schema()
        conn = self._conn()
        total = int(
            conn.execute(
                "SELECT COUNT(*) AS c FROM ai_feedback WHERE is_helpful = 0"
            ).fetchone()["c"]
        )
        rows = conn.execute(
            """SELECT f.id, f.message_id, f.session_id, f.intent,
                      f.question, f.created_at, h.content
               FROM ai_feedback f
               INNER JOIN ai_history h ON h.id = f.message_id
               WHERE f.is_helpful = 0
               ORDER BY datetime(f.created_at) DESC
               LIMIT ? OFFSET ?""",
            (limit, offset),
        ).fetchall()
        items = [
            {
                "feedback_id": r["id"],
                "message_id": r["message_id"],
                "session_id": r["session_id"],
                "intent": r["intent"],
                "question": r["question"],
                "created_at": r["created_at"],
                "answer": r["content"],
            }
            for r in rows
        ]
        return {"total": total, "limit": limit, "offset": offset, "items": items}


def get_sqlite_session_store() -> SqliteSessionStore:
    return SqliteSessionStore.get_instance()
