"""
app/core/session_manager.py — 会话历史管理

职责：
  - 创建新会话（生成 session_id，即 UUID）
  - 按 session_id 查询会话历史（来自 ai_history）
  - 流式写入：append_content 追加 content、update_stream_done 标记完成
  - 流式续写：get_last_message、get_message_by_id
  - 调用 MainQAAgent 处理问答（MainQAAgent 内部调用 Graph Agent）
  - 将 user + assistant 消息写入 ai_history

单条 message 字段：
  - role: user / assistant / system / tool
  - content: 消息正文
  - intent / entity: 意图识别结果（仅 assistant 角色）
  - sources: 溯源信息 JSON（仅 assistant 角色）
  - token_count: 总 token 数（流式结束后更新）
  - sent_offset: 已发送给客户端的字符偏移量（流式续写用）
  - streaming_done: 流式是否已完成（FALSE=正在生成中）
"""

import asyncio
import json
import uuid
from typing import Optional
from config import settings


class SessionManager:
    MAX_TURNS_PER_SESSION = settings.SESSION_MAX_TURNS
    _initialized = False

    def __init__(self):
        self._main_agent = None


    def _use_sqlite(self) -> bool:
        backend = (getattr(settings, "SESSION_BACKEND", "sqlite") or "sqlite").strip().lower()
        return backend != "mysql"

    async def ensure_table(self):
        """确保会话表存在（默认 SQLite；可选 MySQL）。"""
        import logging
        if self._use_sqlite():
            from app.db.sqlite_session_store import get_sqlite_session_store
            await asyncio.to_thread(get_sqlite_session_store().ensure_schema)
            logging.info("[SessionManager] using local SQLite session store")
            return

        from app.db.mysql_client import get_pool
        pool = await get_pool()
        try:
            async with pool.acquire() as conn:
                async with conn.cursor() as cur:
                    if not SessionManager._initialized:
                        await cur.execute("""
                            CREATE TABLE IF NOT EXISTS ai_history (
                                id BIGINT AUTO_INCREMENT PRIMARY KEY,
                                session_id VARCHAR(64) NOT NULL,
                                role VARCHAR(20) NOT NULL,
                                content LONGTEXT,
                                tool_name VARCHAR(100),
                                tool_input JSON,
                                tool_output LONGTEXT,
                                intent VARCHAR(100),
                                entity JSON,
                                sources JSON,
                                token_count INT DEFAULT 0,
                                sent_offset INT DEFAULT 0,
                                streaming_done BOOLEAN DEFAULT FALSE,
                                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                                INDEX idx_session_id (session_id),
                                INDEX idx_created_at (created_at)
                            ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4
                        """)
                        logging.info("[SessionManager] ai_history table ensured")
                        SessionManager._initialized = True
                    try:
                        await cur.execute(
                            "ALTER TABLE ai_history ADD COLUMN sources JSON NULL"
                        )
                    except Exception:
                        pass
                    await cur.execute("""
                        CREATE TABLE IF NOT EXISTS ai_feedback (
                            id BIGINT AUTO_INCREMENT PRIMARY KEY,
                            message_id BIGINT NOT NULL,
                            session_id VARCHAR(64) NOT NULL,
                            is_helpful BOOLEAN NOT NULL,
                            intent VARCHAR(100),
                            question TEXT,
                            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                            UNIQUE KEY uk_message_id (message_id),
                            INDEX idx_session_id (session_id),
                            INDEX idx_is_helpful (is_helpful),
                            INDEX idx_created_at (created_at)
                        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4
                    """)
        except Exception as e:
            logging.error(f"[SessionManager] Failed to ensure table: {e}")

    def _get_main_agent(self):
        if self._main_agent is None:
            from app.agents.main_qa_agent import get_main_qa_agent
            self._main_agent = get_main_qa_agent()
        return self._main_agent

    async def create_session(self) -> dict:
        session_id = str(uuid.uuid4())
        return {"session_id": session_id}

    async def delete_session(self, session_id: str) -> dict:
        from app.core.query_memory import get_query_memory
        import logging
        logging.info(f"[SessionManager] Deleting session: {session_id}")
        get_query_memory().clear_session(session_id)

        if self._use_sqlite():
            from app.db.sqlite_session_store import get_sqlite_session_store
            deleted_count = await asyncio.to_thread(
                get_sqlite_session_store().delete_session, session_id
            )
            return {"session_id": session_id, "deleted_count": deleted_count}

        from app.db.mysql_client import get_pool
        pool = await get_pool()
        async with pool.acquire() as conn:
            async with conn.cursor() as cur:
                await cur.execute("DELETE FROM ai_feedback WHERE session_id = %s", (session_id,))
                await cur.execute("DELETE FROM ai_history WHERE session_id = %s", (session_id,))
                return {"session_id": session_id, "deleted_count": cur.rowcount}

    async def save_message(
        self,
        session_id: str,
        role: str,
        content: str,
        tool_name: Optional[str] = None,
        tool_input: Optional[dict] = None,
        tool_output: Optional[str] = None,
        intent: Optional[str] = None,
        entity: Optional[str] = None,
    ) -> int:
        if self._use_sqlite():
            from app.db.sqlite_session_store import get_sqlite_session_store
            return await asyncio.to_thread(
                get_sqlite_session_store().save_message,
                session_id, role, content, tool_name, tool_input, tool_output, intent, entity,
            )

        from app.db.mysql_client import get_pool
        pool = await get_pool()
        async with pool.acquire() as conn:
            async with conn.cursor() as cur:
                await cur.execute(
                    """INSERT INTO ai_history
                       (session_id, role, content, tool_name, tool_input, tool_output,
                        intent, entity, token_count, sent_offset, streaming_done)
                       VALUES (%s, %s, %s, %s, %s, %s, %s, %s, 0, 0, FALSE)""",
                    (
                        session_id, role, content, tool_name,
                        json.dumps(tool_input, ensure_ascii=False) if tool_input else None,
                        tool_output, intent, entity,
                    ),
                )
                await conn.commit()
                await cur.execute("SELECT LAST_INSERT_ID()")
                row = await cur.fetchone()
                return row[0] if row else 0

    async def append_content(self, session_id: str, message_id: int, new_chunk: str) -> None:
        if self._use_sqlite():
            from app.db.sqlite_session_store import get_sqlite_session_store
            await asyncio.to_thread(
                get_sqlite_session_store().append_content, session_id, message_id, new_chunk
            )
            return
        from app.db.mysql_client import get_pool
        pool = await get_pool()
        async with pool.acquire() as conn:
            async with conn.cursor() as cur:
                await cur.execute(
                    """UPDATE ai_history
                       SET content = CONCAT(IFNULL(content, ''), %s),
                           sent_offset = LENGTH(CONCAT(IFNULL(content, ''), %s))
                       WHERE id = %s AND session_id = %s""",
                    (new_chunk, new_chunk, message_id, session_id),
                )
                await conn.commit()

    async def update_stream_done(
        self,
        session_id: str,
        message_id: int,
        total_tokens: int = 0,
        sources: Optional[list] = None,
        entity: Optional[dict] = None,
    ) -> None:
        if self._use_sqlite():
            from app.db.sqlite_session_store import get_sqlite_session_store
            await asyncio.to_thread(
                get_sqlite_session_store().update_stream_done,
                session_id, message_id, total_tokens, sources, entity,
            )
            return
        from app.db.mysql_client import get_pool
        sources_json = json.dumps(sources, ensure_ascii=False) if sources else None
        entity_json = json.dumps(entity, ensure_ascii=False) if entity else None
        pool = await get_pool()
        async with pool.acquire() as conn:
            async with conn.cursor() as cur:
                await cur.execute(
                    """UPDATE ai_history
                       SET streaming_done = TRUE,
                           token_count = %s,
                           sent_offset = LENGTH(content),
                           sources = %s,
                           entity = %s
                       WHERE id = %s AND session_id = %s""",
                    (total_tokens, sources_json, entity_json, message_id, session_id),
                )
                await conn.commit()

    @staticmethod
    def _parse_entity_field(raw) -> dict:
        if not raw:
            return {}
        if isinstance(raw, dict):
            return raw
        if isinstance(raw, str):
            try:
                parsed = json.loads(raw)
                return parsed if isinstance(parsed, dict) else {}
            except (json.JSONDecodeError, TypeError):
                return {}
        return {}

    @staticmethod
    def _parse_sources_field(raw) -> list:
        if not raw:
            return []
        if isinstance(raw, list):
            return raw
        if isinstance(raw, str):
            try:
                parsed = json.loads(raw)
                return parsed if isinstance(parsed, list) else []
            except (json.JSONDecodeError, TypeError):
                return []
        return []

    async def get_last_message(self, session_id: str) -> Optional[dict]:
        if self._use_sqlite():
            from app.db.sqlite_session_store import get_sqlite_session_store
            return await asyncio.to_thread(get_sqlite_session_store().get_last_message, session_id)
        from app.db.mysql_client import get_pool
        pool = await get_pool()
        async with pool.acquire() as conn:
            async with conn.cursor() as cur:
                await cur.execute(
                    """SELECT id, role, content, sent_offset, streaming_done
                       FROM ai_history WHERE session_id = %s
                       ORDER BY created_at DESC, id DESC LIMIT 1""",
                    (session_id,),
                )
                row = await cur.fetchone()
                if not row:
                    return None
                columns = [c[0] for c in cur.description]
                return dict(zip(columns, row))

    async def get_message_by_id(self, session_id: str, message_id: int) -> Optional[dict]:
        if self._use_sqlite():
            from app.db.sqlite_session_store import get_sqlite_session_store
            return await asyncio.to_thread(
                get_sqlite_session_store().get_message_by_id, session_id, message_id
            )
        from app.db.mysql_client import get_pool
        pool = await get_pool()
        async with pool.acquire() as conn:
            async with conn.cursor() as cur:
                await cur.execute(
                    """SELECT id, role, content, sent_offset, streaming_done
                       FROM ai_history WHERE id = %s AND session_id = %s""",
                    (message_id, session_id),
                )
                row = await cur.fetchone()
                if not row:
                    return None
                columns = [c[0] for c in cur.description]
                return dict(zip(columns, row))

    async def _get_feedback_map(self, session_id: str) -> dict:
        if self._use_sqlite():
            from app.db.sqlite_session_store import get_sqlite_session_store
            return await asyncio.to_thread(get_sqlite_session_store().get_feedback_map, session_id)
        from app.db.mysql_client import get_pool
        pool = await get_pool()
        async with pool.acquire() as conn:
            async with conn.cursor() as cur:
                await cur.execute(
                    """SELECT message_id, is_helpful FROM ai_feedback WHERE session_id = %s""",
                    (session_id,),
                )
                rows = await cur.fetchall()
                return {int(row[0]): bool(row[1]) for row in rows}

    async def save_feedback(
        self,
        message_id: int,
        is_helpful: bool,
        session_id: Optional[str] = None,
    ) -> dict:
        if self._use_sqlite():
            from app.db.sqlite_session_store import get_sqlite_session_store
            return await asyncio.to_thread(
                get_sqlite_session_store().save_feedback, message_id, is_helpful, session_id
            )
        from app.db.mysql_client import get_pool
        pool = await get_pool()
        async with pool.acquire() as conn:
            async with conn.cursor() as cur:
                await cur.execute(
                    """SELECT id, session_id, role, intent FROM ai_history WHERE id = %s""",
                    (message_id,),
                )
                row = await cur.fetchone()
                if not row:
                    raise ValueError("消息不存在")
                msg_id, msg_session_id, role, intent = row
                if role != "assistant":
                    raise ValueError("仅可对助手回答提交反馈")
                if session_id and session_id != msg_session_id:
                    raise ValueError("会话不匹配")
                await cur.execute(
                    """SELECT content FROM ai_history
                       WHERE session_id = %s AND role = 'user' AND id < %s
                       ORDER BY id DESC LIMIT 1""",
                    (msg_session_id, message_id),
                )
                question_row = await cur.fetchone()
                question = question_row[0] if question_row else None
                await cur.execute(
                    """INSERT INTO ai_feedback
                           (message_id, session_id, is_helpful, intent, question)
                       VALUES (%s, %s, %s, %s, %s)
                       ON DUPLICATE KEY UPDATE
                           is_helpful = VALUES(is_helpful),
                           intent = VALUES(intent),
                           question = VALUES(question)""",
                    (message_id, msg_session_id, is_helpful, intent, question),
                )
                return {
                    "message_id": message_id,
                    "is_helpful": is_helpful,
                    "feedback": "helpful" if is_helpful else "inaccurate",
                }

    async def get_history(self, session_id: str, max_turns: Optional[int] = None) -> list:
        limit = (max_turns or self.MAX_TURNS_PER_SESSION) * 2
        feedback_map = await self._get_feedback_map(session_id)

        if self._use_sqlite():
            from app.db.sqlite_session_store import get_sqlite_session_store
            rows = await asyncio.to_thread(
                get_sqlite_session_store().get_history_rows, session_id, limit
            )
            result = []
            for item in rows:
                item = dict(item)
                if "streaming_done" in item:
                    item["streaming_done"] = bool(item["streaming_done"])
                if item.get("role") == "assistant":
                    item["sources"] = self._parse_sources_field(item.get("sources"))
                    meta = self._parse_entity_field(item.get("entity"))
                    if meta:
                        item["has_kg_facts"] = bool(meta.get("has_kg_facts"))
                        item["has_llm_content"] = bool(meta.get("has_llm_content"))
                    msg_id = item.get("id")
                    if msg_id in feedback_map:
                        item["feedback"] = "helpful" if feedback_map[msg_id] else "inaccurate"
                else:
                    item.pop("sources", None)
                result.append(item)
            return result

        from app.db.mysql_client import get_pool
        pool = await get_pool()
        async with pool.acquire() as conn:
            async with conn.cursor() as cur:
                await cur.execute(
                    """SELECT id, role, content, intent, entity, streaming_done, sources, created_at
                       FROM ai_history
                       WHERE session_id = %s AND role IN ('user', 'assistant')
                       ORDER BY created_at ASC, id ASC
                       LIMIT %s""",
                    (session_id, limit),
                )
                rows = await cur.fetchall()
                columns = [c[0] for c in cur.description]
                result = []
                for row in rows:
                    item = dict(zip(columns, row))
                    if item.get("role") == "assistant":
                        item["sources"] = self._parse_sources_field(item.get("sources"))
                        meta = self._parse_entity_field(item.get("entity"))
                        if meta:
                            item["has_kg_facts"] = bool(meta.get("has_kg_facts"))
                            item["has_llm_content"] = bool(meta.get("has_llm_content"))
                        msg_id = item.get("id")
                        if msg_id in feedback_map:
                            item["feedback"] = (
                                "helpful" if feedback_map[msg_id] else "inaccurate"
                            )
                    else:
                        item.pop("sources", None)
                    result.append(item)
                return result

    async def get_feedback_stats(self, days: int = 30, top_n: int = 20) -> dict:
        if self._use_sqlite():
            from app.db.sqlite_session_store import get_sqlite_session_store
            return await asyncio.to_thread(
                get_sqlite_session_store().get_feedback_stats, days, top_n
            )
        from app.db.mysql_client import get_pool
        pool = await get_pool()
        async with pool.acquire() as conn:
            async with conn.cursor() as cur:
                await cur.execute(
                    """SELECT COUNT(*) AS total,
                              SUM(CASE WHEN is_helpful THEN 1 ELSE 0 END) AS helpful,
                              SUM(CASE WHEN NOT is_helpful THEN 1 ELSE 0 END) AS inaccurate
                       FROM ai_feedback
                       WHERE created_at >= DATE_SUB(NOW(), INTERVAL %s DAY)""",
                    (days,),
                )
                row = await cur.fetchone()
                total = int(row[0] or 0)
                helpful = int(row[1] or 0)
                inaccurate = int(row[2] or 0)
                rate = round(inaccurate / total, 4) if total else 0.0
                await cur.execute(
                    """SELECT COALESCE(intent, 'UNKNOWN') AS intent,
                              SUM(CASE WHEN NOT is_helpful THEN 1 ELSE 0 END) AS inaccurate_count,
                              COUNT(*) AS total_count
                       FROM ai_feedback
                       WHERE created_at >= DATE_SUB(NOW(), INTERVAL %s DAY)
                       GROUP BY COALESCE(intent, 'UNKNOWN')
                       HAVING inaccurate_count > 0
                       ORDER BY inaccurate_count DESC, total_count DESC""",
                    (days,),
                )
                by_intent = [
                    {"intent": r[0], "inaccurate_count": int(r[1]), "total_count": int(r[2])}
                    for r in await cur.fetchall()
                ]
                await cur.execute(
                    """SELECT question, COALESCE(intent, 'UNKNOWN'), COUNT(*) AS cnt, MAX(created_at)
                       FROM ai_feedback
                       WHERE is_helpful = 0 AND question IS NOT NULL AND TRIM(question) != ''
                         AND created_at >= DATE_SUB(NOW(), INTERVAL %s DAY)
                       GROUP BY question, COALESCE(intent, 'UNKNOWN')
                       ORDER BY cnt DESC, MAX(created_at) DESC LIMIT %s""",
                    (days, top_n),
                )
                top_questions = [
                    {
                        "question": r[0],
                        "intent": r[1],
                        "count": int(r[2]),
                        "latest_at": r[3].isoformat() if r[3] else None,
                    }
                    for r in await cur.fetchall()
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

    async def get_inaccurate_feedback_list(self, limit: int = 50, offset: int = 0) -> dict:
        if self._use_sqlite():
            from app.db.sqlite_session_store import get_sqlite_session_store
            return await asyncio.to_thread(
                get_sqlite_session_store().get_inaccurate_feedback_list, limit, offset
            )
        from app.db.mysql_client import get_pool
        pool = await get_pool()
        async with pool.acquire() as conn:
            async with conn.cursor() as cur:
                await cur.execute("SELECT COUNT(*) FROM ai_feedback WHERE is_helpful = 0")
                total = int((await cur.fetchone())[0])
                await cur.execute(
                    """SELECT f.id, f.message_id, f.session_id, f.intent,
                              f.question, f.created_at, h.content
                       FROM ai_feedback f
                       INNER JOIN ai_history h ON h.id = f.message_id
                       WHERE f.is_helpful = 0
                       ORDER BY f.created_at DESC LIMIT %s OFFSET %s""",
                    (limit, offset),
                )
                items = [
                    {
                        "feedback_id": r[0],
                        "message_id": r[1],
                        "session_id": r[2],
                        "intent": r[3],
                        "question": r[4],
                        "created_at": r[5].isoformat() if r[5] else None,
                        "answer": r[6],
                    }
                    for r in await cur.fetchall()
                ]
                return {"total": total, "limit": limit, "offset": offset, "items": items}

    async def process_with_history(self, question: str, session_id: str) -> dict:
        """
        带会话历史的非流式问答（本地 SQLite / 可选 MySQL）。
        使用与 WebSocket 相同的 QA Agent 流水线，保证多轮上下文一致。
        """
        from app.models.schemas import SourceInfo
        import logging

        history = await self.get_history(session_id)
        await self.save_message(session_id, "user", question)
        assistant_id = await self.save_message(session_id, "assistant", "")

        collected = {"content": "", "sources": [], "intent_label": "LEGAL_QA",
                     "has_kg_facts": False, "has_llm_content": False}

        async def token_callback(token: str):
            collected["content"] += token
            await self.append_content(session_id, assistant_id, token)

        async def done_callback(result):
            if not result:
                return
            collected["content"] = result.get("content") or collected["content"]
            collected["sources"] = result.get("sources") or []
            collected["intent_label"] = result.get("intent_label") or "LEGAL_QA"
            collected["has_kg_facts"] = bool(result.get("has_kg_facts"))
            collected["has_llm_content"] = bool(result.get("has_llm_content"))
            await self.update_stream_done(
                session_id,
                assistant_id,
                sources=collected["sources"],
                entity={
                    "has_kg_facts": collected["has_kg_facts"],
                    "has_llm_content": collected["has_llm_content"],
                },
            )

        from app.agents.main_agent import run_main_agent
        await run_main_agent(
            question=question,
            history=history,
            token_callback=token_callback,
            done_callback=done_callback,
            session_id=session_id,
        )

        sources = [
            SourceInfo(
                museum_name=s.get("museum", "未知法规"),
                detail_url=s.get("url", ""),
                object_id=s.get("object_id", ""),
                image_url=s.get("image_url"),
                accession_number=s.get("accession_number"),
            )
            for s in collected["sources"]
        ]
        logging.info(
            "[SessionManager] process_with_history done: chars=%d sources=%d",
            len(collected["content"]),
            len(sources),
        )
        return {
            "answer_id": str(assistant_id),
            "question": question,
            "answer": collected["content"],
            "intent": collected["intent_label"],
            "entity": "",
            "sources": sources,
            "has_kg_facts": collected["has_kg_facts"],
            "has_llm_content": collected["has_llm_content"],
            "not_found": not collected["has_kg_facts"]
            and not bool((collected["content"] or "").strip()),
        }

    async def process_streaming(
        self,
        question: str,
        session_id: str,
        ws_session_id: str,
        stop_event: Optional[asyncio.Event] = None,
    ) -> int:
        """
        流式问答流程（用于 WebSocket 接口）。

        流程：
          1. 保存 user 消息
          2. 保存空的 assistant 消息（streaming_done=FALSE）
          3. 使用 Main Agent 调用 Graph Agent + 答案润色
          4. token 实时推送 WS；完成后更新数据库
          5. 返回 assistant 消息的 database id（供 cursor 使用）
        """
        from app.core.ws_manager import get_ws_manager
        import logging

        logging.info(f"[SessionManager] process_streaming called for session: {session_id}")

        ws_manager = get_ws_manager()

        await self.save_message(session_id=session_id, role="user", content=question)

        msg_id = await self.save_message(
            session_id=session_id, role="assistant",
            content="", intent="MAIN_AGENT",
        )

        history = await self.get_history(session_id)

        async def agent_step_callback(step_type: str, content: str, tool_name: str = ""):
            try:
                await ws_manager.send_agent_step(
                    session_id=ws_session_id,
                    content=content,
                    step_type=step_type,
                    tool_name=tool_name or None,
                )
            except Exception:
                pass

        async def token_callback(token: str):
            nonlocal msg_id
            try:
                await ws_manager.send_chunk(
                    session_id=ws_session_id,
                    message_id=msg_id,
                    chunk=token,
                    done=False,
                )
                stream_buffer.append(token)
                if len(stream_buffer.pending) >= settings.STREAM_DB_FLUSH_CHARS:
                    await stream_buffer.flush()
            except Exception:
                pass

        class _StreamBuffer:
            def __init__(self):
                self.pending = ""

            def append(self, token: str):
                self.pending += token

            async def flush(self):
                if not self.pending:
                    return
                chunk = self.pending
                self.pending = ""
                await self._append(ws_session_id, msg_id, chunk)

            async def _append(self, sid: str, mid: int, text: str):
                await self_outer.append_content(sid, mid, text)

        self_outer = self
        stream_buffer = _StreamBuffer()

        async def done_callback(result: Optional[dict]):
            nonlocal msg_id
            import logging
            try:
                await stream_buffer.flush()
                if result is None:
                    await ws_manager.send_to_session(ws_session_id, {
                        "type": "error",
                        "message": "用户已停止生成",
                    })
                    return

                full_content = result.get("content", "")
                sources = result.get("sources", [])
                intent_label = result.get("intent_label", "UNKNOWN")
                has_kg_facts = bool(result.get("has_kg_facts"))
                has_llm_content = bool(result.get("has_llm_content"))

                cursor = f"{msg_id}+{len(full_content)}"
                await ws_manager.send_to_session(ws_session_id, {
                    "type": "done",
                    "message_id": msg_id,
                    "content": full_content,
                    "has_kg_facts": has_kg_facts,
                    "has_llm_content": has_llm_content,
                    "cursor": cursor,
                    "intent": intent_label,
                    "sources": sources,
                })
                try:
                    await self.update_stream_done(
                        ws_session_id,
                        msg_id,
                        sources=sources,
                        entity={
                            "has_kg_facts": has_kg_facts,
                            "has_llm_content": has_llm_content,
                        },
                    )
                    logging.info(
                        "[SessionManager] saved %d source(s) for message %s",
                        len(sources), msg_id,
                    )
                except Exception as db_err:
                    logging.error("[SessionManager] save sources failed: %s", db_err)
            except Exception as e:
                logging.error(f"[SessionManager] done_callback error: {e}")
                pass

        try:
            from app.agents.main_agent import run_main_agent
            await run_main_agent(
                question=question,
                history=history,
                token_callback=token_callback,
                done_callback=done_callback,
                session_id=ws_session_id,
                agent_step_callback=agent_step_callback,
                stop_event=stop_event,
            )
        except Exception as e:
            import logging
            logging.error(f"[SessionManager] Error in process_streaming: {e}", exc_info=True)
            await stream_buffer.flush()
            err_msg = str(e) or "问答服务异常，请稍后重试"
            await ws_manager.send_error(ws_session_id, err_msg)
            try:
                await self.update_stream_done(
                    ws_session_id,
                    msg_id,
                    sources=[],
                    entity={"has_kg_facts": False, "has_llm_content": False},
                )
            except Exception:
                pass

        return msg_id

    async def _flush_chunk(self, session_id: str, message_id: int, chunk: str) -> None:
        try:
            await self.append_content(session_id, message_id, chunk)
        except Exception:
            pass


_session_manager: Optional[SessionManager] = None


def get_session_manager() -> SessionManager:
    global _session_manager
    if _session_manager is None:
        _session_manager = SessionManager()
    return _session_manager