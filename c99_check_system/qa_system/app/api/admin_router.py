"""
app/api/admin_router.py — 管理员接口

  POST /api/qa/admin/login               — 管理员登录（密码）
  GET  /api/qa/admin/feedback/stats      — 高频错误统计（需登录）
  GET  /api/qa/admin/feedback/inaccurate — 不准确反馈审核列表（需登录）
"""

from fastapi import APIRouter, Depends, Header, HTTPException, Query

from app.api.admin_auth import admin_password, require_admin
from app.core.admin_session import create_admin_session, revoke_admin_session
from app.core.session_manager import get_session_manager
from app.models.schemas import (
    AdminLoginRequest,
    AdminLoginResponse,
    FeedbackStatsResponse,
    InaccurateFeedbackListResponse,
)

router = APIRouter(prefix="/admin", tags=["admin"])
protected = APIRouter(dependencies=[Depends(require_admin)])
session_manager = get_session_manager()


@router.post("/login", response_model=AdminLoginResponse)
async def admin_login(request: AdminLoginRequest):
    """管理员登录：校验密码后返回会话 token（供 X-Admin-Session 头使用）。"""
    expected_key = admin_password()
    if not expected_key:
        raise HTTPException(
            status_code=503,
            detail="管理员登录未启用，请在服务端配置 ADMIN_API_KEY",
        )
    if request.password != expected_key:
        raise HTTPException(status_code=401, detail="密码错误")
    token, expires_in = create_admin_session()
    return AdminLoginResponse(token=token, expires_in=expires_in)


@router.post("/logout")
async def admin_logout(
    x_admin_session: str | None = Header(default=None, alias="X-Admin-Session"),
):
    revoke_admin_session(x_admin_session)
    return {"status": "ok"}


@protected.get("/feedback/stats", response_model=FeedbackStatsResponse)
async def feedback_stats(
    days: int = Query(30, ge=1, le=365, description="统计最近 N 天"),
    top_n: int = Query(20, ge=1, le=100, description="返回高频错误问题条数"),
):
    try:
        return await session_manager.get_feedback_stats(days=days, top_n=top_n)
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"统计查询失败: {str(e)}")


@protected.get("/feedback/inaccurate", response_model=InaccurateFeedbackListResponse)
async def inaccurate_feedback_list(
    limit: int = Query(50, ge=1, le=200),
    offset: int = Query(0, ge=0),
):
    try:
        return await session_manager.get_inaccurate_feedback_list(
            limit=limit,
            offset=offset,
        )
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"列表查询失败: {str(e)}")


router.include_router(protected)
