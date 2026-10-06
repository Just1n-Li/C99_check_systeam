"""
main.py — 应用入口
启动 FastAPI 服务，注册所有路由。
运行方式: uvicorn main:app --reload --port 8000

前端由独立的 chat-web/（Vue 3 + Vite）服务；本服务只暴露后端 API 与 WebSocket。
"""

from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from app.api.qa_router import router as qa_router
from app.api.admin_router import router as admin_router
from app.api.ws_router import register_ws_router
from config import settings

app = FastAPI(
    title="法律法规智能问答系统",
    description="基于本地大语言模型 + FAISS RAG + 多 Agent 任务调度的法律法规问答服务",
    version="2.0.0",
)

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_methods=["*"],
    allow_headers=["*"],
)

app.include_router(qa_router, prefix="/api/qa")
app.include_router(admin_router, prefix="/api/qa")
register_ws_router(app)


@app.on_event("startup")
async def startup():
    import logging
    from config import settings, ENV_FILE

    logging.info(
        "[Startup] env=%s faiss=%s session_backend=%s ollama=%s llm=%s model=%s key_set=%s",
        ENV_FILE,
        settings.FAISS_INDEX_DIR,
        getattr(settings, "SESSION_BACKEND", "sqlite"),
        settings.USE_OLLAMA,
        settings.resolved_llm_base_url(),
        settings.LLM_MODEL_NAME,
        bool(settings.resolved_llm_api_key()),
    )

    try:
        from app.retrieval.faiss_retriever import get_faiss_retriever
        ok = get_faiss_retriever().load()
        logging.info(
            "[Startup] FAISS load: %s",
            "ok" if ok else "missing (run scripts/build_faiss_index.py)",
        )
    except Exception as e:
        logging.warning("[Startup] FAISS preload skipped: %s", e)

    try:
        from app.core.session_manager import get_session_manager
        await get_session_manager().ensure_table()
    except Exception as e:
        logging.warning("[Startup] session store ensure failed: %s", e)


@app.on_event("shutdown")
async def shutdown():
    backend = (getattr(settings, "SESSION_BACKEND", "sqlite") or "sqlite").strip().lower()
    if backend == "mysql" or settings.ENABLE_MYSQL:
        try:
            from app.db.mysql_client import MySQLClient
            await MySQLClient.close_pool()
        except Exception:
            pass


@app.get("/")
async def index():
    """后端根路径。前端由 chat-web/ 独立提供。"""
    return {
        "service": "legal-qa-backend",
        "version": "2.0.0",
        "docs": "/docs",
        "api": "/api/qa",
        "admin_api": "/api/qa/admin (requires X-Admin-Key)",
        "websocket": "/api/qa/ws?session_id=...",
        "agents": ["retriever", "summarizer", "qa"],
        "retrieval": "FAISS",
        "frontend_hint": "前端由 ../chat-web/ 启动 (默认 http://localhost:5173)",
    }


@app.get("/health")
def health_check():
    from app.retrieval.faiss_retriever import get_faiss_retriever

    retriever = get_faiss_retriever()
    return {
        "status": "ok",
        "faiss_ready": retriever.is_ready,
        "faiss_index_dir": settings.FAISS_INDEX_DIR,
        "session_backend": getattr(settings, "SESSION_BACKEND", "sqlite"),
        "enable_mysql": settings.ENABLE_MYSQL,
        "use_ollama": settings.USE_OLLAMA,
        "llm_model": settings.LLM_MODEL_NAME,
    }
