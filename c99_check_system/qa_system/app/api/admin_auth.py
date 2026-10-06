"""
管理员接口鉴权。

支持两种方式（二选一）：
  X-Admin-Key: <ADMIN_API_KEY>          — 脚本 / curl 直连
  X-Admin-Session: <login 返回的 token>  — 前端管理页登录后使用

在 .env 中配置 ADMIN_API_KEY、ADMIN_PASSWORD；未配置 ADMIN_API_KEY 时管理接口返回 503。
"""

from fastapi import Header, HTTPException
from config import settings
from app.core.admin_session import validate_admin_session


def admin_password() -> str:
    pwd = settings.ADMIN_PASSWORD.strip()
    if pwd:
        return pwd
    return settings.ADMIN_API_KEY.strip()


async def require_admin(
    x_admin_key: str | None = Header(default=None, alias="X-Admin-Key"),
    x_admin_session: str | None = Header(default=None, alias="X-Admin-Session"),
) -> None:
    expected_key = settings.ADMIN_API_KEY.strip()
    if not expected_key:
        raise HTTPException(
            status_code=503,
            detail="管理员接口未启用，请在服务端配置 ADMIN_API_KEY",
        )
    if x_admin_key and x_admin_key == expected_key:
        return
    if validate_admin_session(x_admin_session):
        return
    raise HTTPException(status_code=401, detail="未登录或会话已过期")
