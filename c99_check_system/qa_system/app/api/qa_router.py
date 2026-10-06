"""
app/api/qa_router.py — 接口层（API Layer）

职责：只做三件事——接收请求、调用核心处理层、返回响应。
不包含任何业务逻辑。

对外暴露的接口（供前端同学对接）：
  POST /api/qa/session        — 创建新会话，返回 session_id
  POST /api/qa/ask           — 提问，获取回答（支持多轮对话）
  GET  /api/qa/history/{session_id}  — 查询会话历史
  POST /api/qa/feedback      — 提交回答质量反馈
"""

import uuid
from fastapi import APIRouter, HTTPException

from app.models.schemas import AskRequest, AskResponse, FeedbackRequest
from app.core.session_manager import get_session_manager

router = APIRouter()

session_manager = get_session_manager()


@router.post("/session", status_code=201)
async def create_session():
    """
    创建新会话，返回 session_id（UUID）。
    """
    result = await session_manager.create_session()
    return result


@router.delete("/session/{session_id}")
async def delete_session(session_id: str):
    """
    删除指定会话，清理数据库中的历史消息。
    """
    try:
        result = await session_manager.delete_session(session_id)
        return result
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"删除会话失败: {str(e)}")


@router.post("/ask", response_model=AskResponse)
async def ask(request: AskRequest):
    """
    主问答接口。

    流程：
      1. 有 session_id → 从数据库加载历史消息，注入 Graph Agent 实现多轮对话
      2. 无 session_id → 从零开始单轮问答
      3. 有 session_id 时将 user + assistant 消息写入 ai_history
      4. 返回带溯源信息的标准化响应

    调用示例（供子系统2/4参考）：
      POST /api/qa/ask
      {"question": "试用期最多可以约定多久？", "session_id": "可选"}
    """
    try:
        if request.session_id:
            result = await session_manager.process_with_history(
                question=request.question,
                session_id=request.session_id,
            )
        else:
            from app.core.qa_engine import QAEngine
            engine = QAEngine()
            result = await engine.process(question=request.question)
        return result
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"问答服务异常: {str(e)}")


@router.get("/history/{session_id}")
async def get_history(session_id: str):
    """
    查询指定会话的历史消息，按时间升序返回。
    """
    try:
        history = await session_manager.get_history(session_id)
        return {"session_id": session_id, "history": history}
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"查询历史失败: {str(e)}")


@router.post("/feedback")
async def feedback(request: FeedbackRequest):
    """
    反馈接口。用户标记回答是否准确。
    is_helpful=True 表示有帮助，False 表示不准确（供人工审核）。
    """
    try:
        result = await session_manager.save_feedback(
            message_id=request.message_id,
            is_helpful=request.is_helpful,
            session_id=request.session_id,
        )
        return {
            "status": "ok",
            "message": "感谢反馈" if request.is_helpful else "已记录，我们将尽快审核",
            **result,
        }
    except ValueError as e:
        raise HTTPException(status_code=400, detail=str(e))
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"反馈提交失败: {str(e)}")