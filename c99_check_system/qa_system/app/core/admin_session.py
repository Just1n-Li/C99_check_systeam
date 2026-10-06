"""管理员登录会话（内存 token，重启后失效）。"""

import time
import uuid

_TTL_SECONDS = 8 * 3600
_sessions: dict[str, float] = {}


def create_admin_session() -> tuple[str, int]:
    token = str(uuid.uuid4())
    _sessions[token] = time.time() + _TTL_SECONDS
    return token, _TTL_SECONDS


def validate_admin_session(token: str | None) -> bool:
    if not token:
        return False
    expires = _sessions.get(token)
    if expires is None:
        return False
    if time.time() > expires:
        _sessions.pop(token, None)
        return False
    return True


def revoke_admin_session(token: str | None) -> None:
    if token:
        _sessions.pop(token, None)
