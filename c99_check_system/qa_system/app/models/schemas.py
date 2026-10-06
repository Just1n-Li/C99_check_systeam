# app/models/schemas.py

from pydantic import BaseModel
from typing import Optional

# ══════════════════════════════════════════════
# 法规分段模型 —— 对应 FAISS meta / 清洗交付 JSON
# ══════════════════════════════════════════════

class LegalChunk(BaseModel):
    """
    法律法规文本分段。

    兼容清洗侧 chunks.jsonl 字段：
      id / law_name / article / text / status / source_url / source_file
    内部统一为 chunk_id + source。
    """

    chunk_id: str
    law_name: str
    article: Optional[str] = None
    text: str
    source: Optional[str] = None
    status: Optional[str] = None
    source_file: Optional[str] = None
    law_id: Optional[str] = None
    score: Optional[float] = None


# ══════════════════════════════════════════════
# 请求模型（接口形态保持不变，供前端对接）
# ══════════════════════════════════════════════

class AskRequest(BaseModel):
    question: str
    session_id: Optional[str] = None

class FeedbackRequest(BaseModel):
    message_id: int
    session_id: Optional[str] = None
    is_helpful: bool


class FeedbackStatsSummary(BaseModel):
    total_feedback: int
    helpful_count: int
    inaccurate_count: int
    inaccurate_rate: float


class FeedbackIntentStat(BaseModel):
    intent: str
    inaccurate_count: int
    total_count: int


class FeedbackQuestionStat(BaseModel):
    question: str
    count: int
    intent: Optional[str] = None
    latest_at: Optional[str] = None


class FeedbackStatsResponse(BaseModel):
    days: int
    summary: FeedbackStatsSummary
    by_intent: list[FeedbackIntentStat]
    top_inaccurate_questions: list[FeedbackQuestionStat]


class InaccurateFeedbackItem(BaseModel):
    feedback_id: int
    message_id: int
    session_id: str
    question: Optional[str] = None
    answer: Optional[str] = None
    intent: Optional[str] = None
    created_at: Optional[str] = None


class InaccurateFeedbackListResponse(BaseModel):
    total: int
    limit: int
    offset: int
    items: list[InaccurateFeedbackItem]


class AdminLoginRequest(BaseModel):
    password: str


class AdminLoginResponse(BaseModel):
    token: str
    expires_in: int


# ══════════════════════════════════════════════
# 响应模型
# ══════════════════════════════════════════════

class SourceInfo(BaseModel):
    """
    答案溯源信息（字段名保持兼容，便于现有前端消费）。

    法律法规语义映射：
      - museum_name      → 法律名称（law_name）
      - detail_url       → 来源链接（北大法宝等）
      - object_id        → 条文编号（article）或 chunk_id
      - accession_number → chunk_id（可选）
      - image_url        → 法规场景通常为空
    """
    museum_name: str
    detail_url: str
    object_id: str
    image_url: Optional[str] = None
    accession_number: Optional[str] = None


class AskResponse(BaseModel):
    answer_id: str
    question: str
    answer: str
    intent: str
    entity: str
    sources: list[SourceInfo]
    has_kg_facts: bool
    has_llm_content: bool
    not_found: bool
