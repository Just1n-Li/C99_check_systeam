"""
多轮对话历史整理与注入。

WebSocket 流式路径在 process_streaming 中会先写入当前 user 消息和空的 assistant 占位，
调用方需用 prepare_chat_history 去掉占位后再注入 Graph / Main Agent。
"""

from __future__ import annotations

from typing import Any, Optional

from config import settings


def prepare_chat_history(
    history: Optional[list[dict[str, Any]]],
    current_question: str,
) -> list[dict[str, str]]:
    """去掉空 assistant 占位与当前轮 user 重复项，只保留已完成的历史轮次。"""
    if not history:
        return []

    cleaned: list[dict[str, str]] = []
    for record in history:
        role = record.get("role")
        content = (record.get("content") or "").strip()
        if role not in ("user", "assistant"):
            continue
        if role == "assistant" and not content:
            continue
        if role == "assistant" and record.get("streaming_done") is False:
            continue
        cleaned.append({"role": role, "content": content})

    q = current_question.strip()
    if cleaned and cleaned[-1]["role"] == "user" and cleaned[-1]["content"].strip() == q:
        cleaned.pop()

    return cleaned


def _truncate(text: str, max_chars: int) -> str:
    text = text.strip()
    if len(text) <= max_chars:
        return text
    return text[: max_chars - 1] + "…"


def format_history_block(
    history: list[dict[str, str]],
    *,
    max_messages: Optional[int] = None,
    max_chars_per_msg: int = 600,
) -> str:
    """格式化为可注入 system prompt 的文本块。"""
    if not history:
        return ""

    limit = max_messages or (settings.SESSION_MAX_TURNS * 2)
    lines: list[str] = []
    for record in history[-limit:]:
        prefix = "用户" if record["role"] == "user" else "助手"
        content = _truncate(record["content"], max_chars_per_msg)
        lines.append(f"{prefix}：{content}")

    return "\n".join(lines)


def build_openai_history_messages(
    history: list[dict[str, str]],
    *,
    max_messages: Optional[int] = None,
    max_chars_per_msg: int = 600,
) -> list[dict[str, str]]:
    """转为 OpenAI 多轮 messages（不含 system / 当前轮）。"""
    if not history:
        return []

    limit = max_messages or (settings.SESSION_MAX_TURNS * 2)
    result: list[dict[str, str]] = []
    for record in history[-limit:]:
        result.append({
            "role": record["role"],
            "content": _truncate(record["content"], max_chars_per_msg),
        })
    return result
