export interface SourceInfo {
  museum: string
  url: string
  object_id: string
  title?: string
  image_url?: string
  accession_number?: string
}

export interface AskResponse {
  answer_id: string
  question: string
  answer: string
  intent: string
  entity: string
  sources: SourceInfo[]
  has_kg_facts: boolean
  has_llm_content: boolean
  not_found: boolean
}

export type WSMessageType =
  | 'connected'
  | 'chunk'
  | 'done'
  | 'error'
  | 'resume_remaining'
  | 'pong'
  | 'agent_step'

export interface WSServerMessage {
  type: WSMessageType
  session_id?: string
  last_message_id?: number
  streaming_done?: boolean
  last_content?: string
  sent_offset?: number
  message_id?: number
  content?: string
  done?: boolean
  remaining?: string
  cursor?: string
  error?: string
  message?: string
  intent?: string
  sources?: SourceInfo[]
  has_kg_facts?: boolean
  has_llm_content?: boolean
  tool_name?: string
  tool_args?: string
  tool_result?: string
  thinking_content?: string
  step_type?: 'reasoning' | 'tool_call' | 'tool_result'
}

export type MessageType = 'text' | 'rag_agent'

export interface AgentStep {
  id: string
  step_type: 'reasoning' | 'tool_call' | 'tool_result'
  content: string
  timestamp: number
  tool_name?: string
}

export type FeedbackType = 'helpful' | 'inaccurate'

export interface ChatMessage {
  id: string
  role: 'user' | 'assistant' | 'tool'
  content: string
  sources?: SourceInfo[]
  has_kg_facts?: boolean
  has_llm_content?: boolean
  streaming?: boolean
  intent?: string
  timestamp: number
  message_type?: MessageType
  tool_name?: string
  agent_steps?: AgentStep[]
  feedback?: FeedbackType
  feedbackSubmitting?: boolean
}

export interface SessionItem {
  id: string
  title: string
  updatedAt: number
  messages?: ChatMessage[]
  cursor?: string
  isStreaming?: boolean
}

export interface FeedbackStatsSummary {
  total_feedback: number
  helpful_count: number
  inaccurate_count: number
  inaccurate_rate: number
}

export interface FeedbackIntentStat {
  intent: string
  inaccurate_count: number
  total_count: number
}

export interface FeedbackQuestionStat {
  question: string
  count: number
  intent?: string | null
  latest_at?: string | null
}

export interface FeedbackStatsResponse {
  days: number
  summary: FeedbackStatsSummary
  by_intent: FeedbackIntentStat[]
  top_inaccurate_questions: FeedbackQuestionStat[]
}

export interface InaccurateFeedbackItem {
  feedback_id: number
  message_id: number
  session_id: string
  question?: string | null
  answer?: string | null
  intent?: string | null
  created_at?: string | null
}

export interface InaccurateFeedbackListResponse {
  total: number
  limit: number
  offset: number
  items: InaccurateFeedbackItem[]
}