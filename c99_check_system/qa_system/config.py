"""
config.py — 全局配置
读取 .env 文件中的所有配置项，整个项目统一从这里导入。
"""

from pathlib import Path

from pydantic import field_validator
from pydantic_settings import BaseSettings, SettingsConfigDict

# 固定从 qa_system 目录加载 .env，避免在错误 cwd 下启动时读不到配置
BASE_DIR = Path(__file__).resolve().parent
ENV_FILE = BASE_DIR / ".env"


def _env_bool(value: object) -> bool:
    """兼容 .env 里 on/off、true/false 等写法。"""
    if isinstance(value, bool):
        return value
    if value is None:
        return False
    return str(value).strip().lower() in {"1", "true", "yes", "on", "y"}


class Settings(BaseSettings):
    model_config = SettingsConfigDict(
        env_file=str(ENV_FILE),
        env_file_encoding="utf-8",
        extra="ignore",
    )

    # ── FAISS 向量检索（C99 标准 RAG）──────────────────────────
    FAISS_INDEX_DIR: str = str(BASE_DIR / "data_c99" / "faiss")
    # C99 条款数据目录（内含 chunks.jsonl，见 data_c99/README.md）
    LAW_DATA_DIR: str = str(BASE_DIR / "data_c99")
    EMBEDDING_MODEL: str = "sentence-transformers/paraphrase-multilingual-MiniLM-L12-v2"
    RAG_TOP_K: int = 5

    # ── 会话存储：默认本地 SQLite（多轮不依赖远程 MySQL）──────
    # sqlite | mysql
    SESSION_BACKEND: str = "sqlite"
    SQLITE_SESSION_PATH: str = str(BASE_DIR / "data" / "sessions.db")

    # ── 会话存储：MySQL 仅当 SESSION_BACKEND=mysql 时使用 ──────
    ENABLE_MYSQL: bool = False
    # 旧文物图谱默认关闭
    ENABLE_NEO4J: bool = False

    # ── Neo4j（遗留，默认关闭）────────────────────────────────
    NEO4J_URI: str = "bolt://127.0.0.1:7687"
    NEO4J_USER: str = "neo4j"
    NEO4J_PASSWORD: str = ""

    # ── MySQL（会话历史）──────────────────────────────────────
    MYSQL_HOST: str = "127.0.0.1"
    MYSQL_PORT: int = 3306
    MYSQL_USER: str = "root"
    MYSQL_PASSWORD: str = ""
    MYSQL_DB: str = "legal_qa"

    # ── 大语言模型（OpenAI 兼容协议；Ollama 也走此协议）──────
    # Ollama 示例：USE_OLLAMA=on, LLM_BASE_URL=http://localhost:11434/v1, LLM_API_KEY=ollama
    USE_OLLAMA: bool = False
    OLLAMA_BASE_URL: str = "http://localhost:11434/v1"
    LLM_BASE_URL: str = "https://api.deepseek.com/v1"
    LLM_API_KEY: str = ""
    LLM_MODEL_NAME: str = "deepseek-chat"

    # ── LLM 生成参数 ──────────────────────────────────────────
    LLM_TEMPERATURE: float = 0.1
    # glm-5.2 等轻量思考模型 8192 足够；纯推理模型（sensenova-*）reasoning 会占输出预算
    LLM_MAX_TOKENS: int = 8192
    LLM_TIMEOUT: int = 60
    LLM_MAX_RETRIES: int = 2

    # ── 会话配置（多轮对话）───────────────────────────────────
    SESSION_MAX_TURNS: int = 8

    # ── 性能调优 ──────────────────────────────────────────────
    RETRIEVER_AGENT_MAX_TURNS: int = 4
    # 兼容旧配置名
    GRAPH_AGENT_MAX_TURNS: int = 4
    STREAM_DB_FLUSH_CHARS: int = 80

    # ── 管理员接口 ────────────────────────────────────────────
    ADMIN_API_KEY: str = ""
    ADMIN_PASSWORD: str = ""

    @field_validator(
        "ENABLE_MYSQL", "ENABLE_NEO4J", "USE_OLLAMA", mode="before"
    )
    @classmethod
    def parse_bool_switch(cls, value: object) -> bool:
        return _env_bool(value)

    def resolved_llm_base_url(self) -> str:
        if self.USE_OLLAMA:
            return self.OLLAMA_BASE_URL.rstrip("/")
        return self.LLM_BASE_URL.rstrip("/")

    def resolved_llm_api_key(self) -> str:
        if self.USE_OLLAMA:
            return self.LLM_API_KEY or "ollama"
        return self.LLM_API_KEY


settings = Settings()
