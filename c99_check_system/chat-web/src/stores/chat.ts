import { defineStore } from 'pinia'
import { ref, computed } from 'vue'
import type { ChatMessage, SessionItem, WSServerMessage, SourceInfo, FeedbackType } from '@/types'
import { createSession, deleteSession as apiDeleteSession, fetchHistory, submitFeedback as apiSubmitFeedback, WSConnection } from '@/api/chat'

const SESSIONS_KEY = 'qa_sessions'
const LAST_ACTIVE_KEY = 'qa_last_active'

function loadSessions(): SessionItem[] {
  try {
    const raw = localStorage.getItem(SESSIONS_KEY)
    if (!raw) return []
    const sessions: SessionItem[] = JSON.parse(raw)
    for (const s of sessions) {
      const hasLiveStream = s.messages?.some((m) => m.streaming)
      if (s.isStreaming && !hasLiveStream) {
        s.isStreaming = false
      }
    }
    return sessions
  } catch {
    return []
  }
}

function saveSessions(sessions: SessionItem[]) {
  localStorage.setItem(SESSIONS_KEY, JSON.stringify(sessions))
}

function getLastActiveSessionId(): string | null {
  return localStorage.getItem(LAST_ACTIVE_KEY)
}

function setLastActiveSessionId(id: string) {
  localStorage.setItem(LAST_ACTIVE_KEY, id)
}

function genId() {
  return `${Date.now()}-${Math.random().toString(36).slice(2, 8)}`
}

function normalizeCompletedAssistantIds(msgs: ChatMessage[]) {
  for (const m of msgs) {
    if (
      m.role === 'assistant'
      && m.message_type !== 'rag_agent'
      && !m.streaming
      && m.content?.trim()
    ) {
      const match = m.id.match(/^stream-(\d+)$/)
      if (match) {
        m.id = `done-${match[1]}`
      }
    }
  }
}

function trimAfterLastUser(msgs: ChatMessage[]): ChatMessage[] {
  let lastUserIdx = -1
  for (let i = msgs.length - 1; i >= 0; i--) {
    if (msgs[i]!.role === 'user') {
      lastUserIdx = i
      break
    }
  }
  if (lastUserIdx === -1) return msgs
  return msgs.slice(0, lastUserIdx + 1)
}

function getLastUserQuestion(msgs: ChatMessage[]): string | null {
  for (let i = msgs.length - 1; i >= 0; i--) {
    if (msgs[i]!.role === 'user' && msgs[i]!.content?.trim()) {
      return msgs[i]!.content.trim()
    }
  }
  return null
}

function isFailedAssistantContent(content: string): boolean {
  const t = content.trim()
  if (!t) return true
  return /生成回答时出错|问答服务异常|未能生成|请稍后重试|Unknown error/i.test(t)
}

function cloneMessages(msgs: ChatMessage[]): ChatMessage[] {
  const cloned = msgs.map((m) => ({ ...m, agent_steps: m.agent_steps?.map((s) => ({ ...s })) }))
  normalizeCompletedAssistantIds(cloned)
  return cloned
}

function dedupeSources(raw: SourceInfo[]): SourceInfo[] {
  const seen = new Set<string>()
  const result: SourceInfo[] = []
  for (const s of raw) {
    const url = (s.url || '').trim()
    if (!url) continue
    const key = url.toLowerCase()
    if (seen.has(key)) continue
    seen.add(key)
    result.push(s)
  }
  return result
}

function normalizeSources(raw: WSServerMessage['sources'] | SourceInfo[] | undefined): SourceInfo[] {
  if (!raw?.length) return []
  const mapped = raw.map((s) => ({
    museum: s.museum ?? (s as { museum_name?: string }).museum_name ?? '来源',
    url: s.url ?? (s as { detail_url?: string }).detail_url ?? '',
    object_id: s.object_id ?? '',
    title: (s as { title?: string }).title,
    image_url: s.image_url,
    accession_number: s.accession_number,
  }))
  return dedupeSources(mapped)
}

function sourcesFromRow(
  row: { sources?: SourceInfo[] | WSServerMessage['sources'] },
  local?: ChatMessage,
  backup?: ChatMessage,
): SourceInfo[] | undefined {
  const fromServer = normalizeSources(row.sources as WSServerMessage['sources'])
  if (fromServer.length > 0) return fromServer
  if (local?.sources?.length) return local.sources
  if (backup?.sources?.length) return backup.sources
  return undefined
}

/** 解析 cursor「messageId+offset」，返回 offset 数值 */
function cursorOffset(cursor?: string): number | null {
  if (!cursor) return null
  const part = cursor.split('+')[1]
  if (!part) return null
  const n = Number(part)
  return Number.isNaN(n) ? null : n
}

/** 避免 resume_remaining 与历史同步重复追加同一段文本 */
function appendResumeContent(existing: ChatMessage, remaining: string, cursor?: string) {
  if (!remaining) return
  const targetLen = cursorOffset(cursor)
  if (targetLen != null && existing.content.length >= targetLen) return
  if (existing.content.endsWith(remaining)) return
  existing.content += remaining
}

export const useChatStore = defineStore('chat', () => {
  const sessions = ref<SessionItem[]>(loadSessions())
  const currentSessionId = ref<string | null>(
    getLastActiveSessionId() ?? (sessions.value[0]?.id ?? null),
  )
  const messages = ref<ChatMessage[]>(
    cloneMessages(sessions.value.find((s) => s.id === currentSessionId.value)?.messages ?? []),
  )
  const wsConn = ref<WSConnection | null>(null)
  /** 当前 WebSocket 绑定的 session，用于丢弃过期连接的消息 */
  const wsBoundSessionId = ref<string | null>(null)
  const isStreaming = ref(false)
  const sidebarOpen = ref(true)
  const error = ref<string | null>(null)
  const retryableQuestion = ref<string | null>(null)

  // 启动时清除 localStorage 残留的 streaming 标记
  for (const s of sessions.value) {
    if (s.isStreaming && !s.messages?.some((m) => m.streaming)) {
      s.isStreaming = false
    }
  }
  saveSessions(sessions.value)

  const canRetry = computed(() =>
    Boolean(retryableQuestion.value) && !isStreaming.value && Boolean(currentSessionId.value),
  )

  function markRetryable(sid: string, question: string | null, errMsg: string) {
    if (question) retryableQuestion.value = question
    error.value = errMsg
    const trimmed = trimAfterLastUser(cloneMessages(getSessionMessages(sid)))
    commitSessionMessages(sid, trimmed)
    setSessionStreaming(sid, false)
  }

  function sessionIndex(sessionId: string) {
    return sessions.value.findIndex((s) => s.id === sessionId)
  }

  function isActiveSession(sessionId: string) {
    return sessionId === currentSessionId.value
  }

  function getSessionMessages(sessionId: string): ChatMessage[] {
    if (isActiveSession(sessionId)) return messages.value
    return sessions.value[sessionIndex(sessionId)]?.messages ?? []
  }

  function commitSessionMessages(sessionId: string, next: ChatMessage[]) {
    const cloned = cloneMessages(next)
    const idx = sessionIndex(sessionId)
    if (idx !== -1) {
      sessions.value[idx]!.messages = cloned
      sessions.value[idx]!.updatedAt = Date.now()
      saveSessions(sessions.value)
    }
    if (isActiveSession(sessionId)) {
      messages.value = cloned
    }
  }

  function setSessionStreaming(sessionId: string, streaming: boolean) {
    const idx = sessionIndex(sessionId)
    if (idx !== -1) {
      sessions.value[idx]!.isStreaming = streaming
      saveSessions(sessions.value)
    }
    if (isActiveSession(sessionId)) {
      isStreaming.value = streaming
    }
  }

  function saveCurrentSessionMessages() {
    if (!currentSessionId.value) return
    commitSessionMessages(currentSessionId.value, messages.value)
  }

  const currentSession = computed(() =>
    sessions.value.find((s) => s.id === currentSessionId.value) ?? null,
  )

  const streamingStatus = computed<{ phase: 'connecting' | 'rag' | 'main'; label: string; detail: string | null } | null>(() => {
    if (!isStreaming.value) return null

    const hasStreamingAssistant = messages.value.some(
      (m) => m.role === 'assistant' && m.streaming && m.message_type !== 'rag_agent',
    )
    if (hasStreamingAssistant) {
      return { phase: 'main', label: '生成测试代码中', detail: null }
    }

    for (let i = messages.value.length - 1; i >= 0; i--) {
      const m = messages.value[i]!
      if (m.message_type === 'rag_agent' && m.agent_steps && m.agent_steps.length > 0) {
        const lastStep = m.agent_steps[m.agent_steps.length - 1]!
        return {
          phase: 'rag',
          label: '检索条款中',
          detail: lastStep.content,
        }
      }
    }

    return { phase: 'connecting', label: '正在连接后端…', detail: null }
  })

  function persistSessionExtra(sessionId: string, extra?: { cursor?: string }) {
    const idx = sessionIndex(sessionId)
    if (idx === -1) return
    if (extra?.cursor) {
      sessions.value[idx]!.cursor = extra.cursor
    }
    sessions.value[idx]!.messages = cloneMessages(getSessionMessages(sessionId))
    sessions.value[idx]!.updatedAt = Date.now()
    saveSessions(sessions.value)
  }

  function findStreamMessage(msgs: ChatMessage[], messageId: number) {
    return msgs.find(
      (m) =>
        m.id === `stream-${messageId}` ||
        m.id === `done-${messageId}` ||
        m.id === 'stream-pending',
    )
  }

  function ensureStreamMessage(msgs: ChatMessage[], messageId: number, initialContent = '') {
    let existing = findStreamMessage(msgs, messageId)
    if (existing) {
      if (initialContent && !existing.content) {
        existing.content = initialContent
      }
      existing.streaming = true
      existing.id = `stream-${messageId}`
      return existing
    }
    existing = {
      id: `stream-${messageId}`,
      role: 'assistant' as const,
      content: initialContent,
      streaming: true,
      timestamp: Date.now(),
    }
    msgs.push(existing)
    return existing
  }

  async function syncHistoryFromServer(sessionId: string): Promise<boolean> {
    try {
      const data = await fetchHistory(sessionId)
      const rows = (data.history ?? []) as Array<{
        id: number
        role: string
        content: string
        streaming_done?: boolean
        sources?: SourceInfo[]
        has_kg_facts?: boolean
        has_llm_content?: boolean
        feedback?: FeedbackType
        created_at?: string
      }>

      const prevMsgs = getSessionMessages(sessionId)
      const localMap = new Map(prevMsgs.map((m) => [m.id, m] as const))
      const backupMap = new Map(prevMsgs.map((m) => [m.id, m] as const))
      const ragMessages = prevMsgs.filter((m) => m.message_type === 'rag_agent')

      if (rows.length === 0) {
        if (isActiveSession(sessionId)) {
          messages.value = []
        }
        commitSessionMessages(sessionId, [])
        setSessionStreaming(sessionId, false)
        return false
      }

      const serverMsgs: ChatMessage[] = rows.map((row) => {
        const msgId = row.streaming_done === false ? `stream-${row.id}` : `done-${row.id}`
        const local = localMap.get(msgId)
        const backup = backupMap.get(msgId)
        const sources = row.role === 'assistant'
          ? sourcesFromRow(row, local, backup)
          : undefined
        return {
          id: msgId,
          role: row.role as ChatMessage['role'],
          content: row.content ?? '',
          streaming: row.streaming_done === false,
          sources,
          has_kg_facts: row.has_kg_facts,
          has_llm_content: row.has_llm_content ?? Boolean(row.content),
          feedback: row.feedback ?? local?.feedback,
          timestamp: row.created_at ? new Date(row.created_at).getTime() : Date.now(),
        }
      })

      const stillStreaming = serverMsgs.some((m) => m.streaming)
      const merged = stillStreaming && ragMessages.length > 0
        ? [...serverMsgs.slice(0, -1), ...ragMessages, serverMsgs[serverMsgs.length - 1]!]
        : serverMsgs

      commitSessionMessages(sessionId, merged)
      setSessionStreaming(sessionId, stillStreaming)
      return stillStreaming
    } catch (e) {
      console.error('[Store] syncHistoryFromServer failed:', e)
      return false
    }
  }

  function closeWs() {
    if (wsConn.value) {
      wsConn.value.close()
      wsConn.value = null
    }
    wsBoundSessionId.value = null
  }

  function createMessageHandler(boundSessionId: string) {
    return (msg: WSServerMessage) => {
      // 丢弃已切换/已关闭的旧连接消息，防止串会话
      if (wsBoundSessionId.value !== boundSessionId) {
        console.warn('[WS] Ignore stale message for session', boundSessionId)
        return
      }

      console.log('[WS] Received:', boundSessionId.slice(0, 8), msg.type, msg)
      const msgs = cloneMessages(getSessionMessages(boundSessionId))

      if (msg.type === 'connected') {
        if (msg.streaming_done === false && msg.last_message_id != null) {
          setSessionStreaming(boundSessionId, true)
          const existing = ensureStreamMessage(msgs, msg.last_message_id)
          if (msg.last_content && !existing.content) {
            existing.content = msg.last_content
          }
          commitSessionMessages(boundSessionId, msgs)
          persistSessionExtra(boundSessionId)
        }
      } else if (msg.type === 'resume_remaining') {
        const cursor = msg.cursor ?? ''
        const msgIdStr = cursor.split('+')[0]
        const messageId = msgIdStr ? Number(msgIdStr) : NaN
        if (!Number.isNaN(messageId)) {
          const existing = ensureStreamMessage(msgs, messageId)
          if (msg.remaining) {
            appendResumeContent(existing, msg.remaining, cursor)
          }
          existing.streaming = msg.done === false
          setSessionStreaming(boundSessionId, msg.done === false)
          commitSessionMessages(boundSessionId, msgs)
          if (msg.done === false && msg.cursor) {
            persistSessionExtra(boundSessionId, { cursor: msg.cursor })
          }
        }
      } else if (msg.type === 'agent_step' && msg.step_type && msg.content) {
        const stepObj = {
          id: genId(),
          step_type: msg.step_type,
          content: msg.content,
          timestamp: Date.now(),
          tool_name: msg.tool_name,
        }
        const lastMsg = msgs[msgs.length - 1]
        if (lastMsg && lastMsg.message_type === 'rag_agent' && lastMsg.agent_steps) {
          lastMsg.agent_steps.push(stepObj)
        } else {
          msgs.push({
            id: genId(),
            role: 'assistant',
            content: '',
            message_type: 'rag_agent',
            agent_steps: [stepObj],
            streaming: false,
            timestamp: Date.now(),
          })
        }
        setSessionStreaming(boundSessionId, true)
        commitSessionMessages(boundSessionId, msgs)
      } else if (msg.type === 'chunk' && msg.message_id !== undefined) {
        setSessionStreaming(boundSessionId, true)
        const existing = findStreamMessage(msgs, msg.message_id)
          ?? ensureStreamMessage(msgs, msg.message_id)
        existing.content += msg.content ?? ''
        existing.streaming = true
        existing.id = `stream-${msg.message_id}`
        commitSessionMessages(boundSessionId, msgs)
        persistSessionExtra(boundSessionId)
      } else if (msg.type === 'done' && msg.message_id !== undefined) {
        const sources = normalizeSources(msg.sources)
        const existing = findStreamMessage(msgs, msg.message_id)
        if (existing) {
          existing.content = msg.content ?? existing.content
          existing.has_kg_facts = msg.has_kg_facts ?? existing.has_kg_facts
          existing.has_llm_content = msg.has_llm_content ?? existing.has_llm_content
          existing.streaming = false
          existing.id = `done-${msg.message_id}`
          existing.sources = sources.length > 0 ? sources : undefined
        } else {
          msgs.push({
            id: `done-${msg.message_id}`,
            role: 'assistant',
            content: msg.content ?? '',
            has_kg_facts: msg.has_kg_facts,
            has_llm_content: msg.has_llm_content,
            sources: sources.length > 0 ? sources : undefined,
            streaming: false,
            timestamp: Date.now(),
          })
        }
        setSessionStreaming(boundSessionId, false)
        commitSessionMessages(boundSessionId, msgs)

        const answerText = msg.content ?? ''
        if (!answerText.trim() || isFailedAssistantContent(answerText)) {
          const question = getLastUserQuestion(msgs)
          if (question && isActiveSession(boundSessionId)) {
            retryableQuestion.value = question
            error.value = answerText.trim()
              ? '回答生成异常，可点击下方重新提问'
              : '未能生成回答，可点击下方重新提问'
            commitSessionMessages(boundSessionId, trimAfterLastUser(msgs))
          }
        } else if (isActiveSession(boundSessionId)) {
          retryableQuestion.value = null
          error.value = null
        }

        const idx = sessionIndex(boundSessionId)
        if (idx !== -1) {
          const userMsg = msgs.find((m) => m.role === 'user')
          const lastAssistant = [...msgs].reverse().find((m) => m.role === 'assistant')
          if (lastAssistant && userMsg) {
            sessions.value[idx]!.title = userMsg.content.slice(0, 40)
          }
          persistSessionExtra(boundSessionId, { cursor: msg.cursor })
        }
      } else if (msg.type === 'error') {
        const errText = msg.error ?? msg.message ?? '请求失败，请重试'
        const question = getLastUserQuestion(msgs)
        if (isActiveSession(boundSessionId)) {
          markRetryable(boundSessionId, question, errText)
        } else {
          setSessionStreaming(boundSessionId, false)
        }
      }
    }
  }

  async function connectWsForSession(sessionId: string) {
    closeWs()

    const session = sessions.value.find((s) => s.id === sessionId)
    let cursor = session?.cursor
    const msgs = getSessionMessages(sessionId)
    const streamingMsg = msgs.find((m) => m.streaming)
    // 历史同步后若已包含 cursor 之前的全部内容，不再带 cursor 重连，避免重复 resume
    if (streamingMsg && cursor) {
      const off = cursorOffset(cursor)
      if (off != null && streamingMsg.content.length >= off) {
        cursor = undefined
      }
    }

    wsBoundSessionId.value = sessionId
    wsConn.value = new WSConnection(
      sessionId,
      createMessageHandler(sessionId),
      () => {
        if (wsBoundSessionId.value === sessionId) {
          wsConn.value = null
          wsBoundSessionId.value = null
        }
      },
      cursor,
    )

    try {
      await wsConn.value.waitOpen()
    } catch (e) {
      console.error('[Store] WS connect failed:', e)
      if (wsBoundSessionId.value === sessionId) {
        wsConn.value = null
        wsBoundSessionId.value = null
      }
      throw e
    }
  }

  async function resumeSession(sessionId?: string) {
    const sid = sessionId ?? currentSessionId.value
    if (!sid) return

    const synced = await syncHistoryFromServer(sid)
    const msgs = getSessionMessages(sid)
    const hasStreamingMsg = msgs.some((m) => m.streaming)
    // 历史同步失败或本地残留 isStreaming 时，若无进行中的消息则解除卡住状态
    if (!hasStreamingMsg && (!synced || !isStreaming.value)) {
      setSessionStreaming(sid, false)
    }
    if (hasStreamingMsg || isStreaming.value) {
      try {
        await connectWsForSession(sid)
      } catch {
        if (!getSessionMessages(sid).some((m) => m.streaming)) {
          setSessionStreaming(sid, false)
        }
      }
    }
  }

  async function sendMessage(content: string) {
    error.value = null
    retryableQuestion.value = null

    // 用户发送新问题时，解除上一轮卡住的状态（WS 会 stop 上一轮任务）
    if (isStreaming.value && currentSessionId.value) {
      const sid = currentSessionId.value
      setSessionStreaming(sid, false)
      const msgs = cloneMessages(getSessionMessages(sid))
      let changed = false
      for (const m of msgs) {
        if (m.streaming) {
          m.streaming = false
          changed = true
        }
      }
      if (changed) commitSessionMessages(sid, msgs)
    }

    if (!currentSessionId.value) {
      const userMsg: ChatMessage = {
        id: genId(),
        role: 'user',
        content,
        timestamp: Date.now(),
      }
      let session_id: string
      try {
        ;({ session_id } = await createSession())
      } catch (e) {
        error.value = e instanceof Error ? e.message : '无法连接后端，请确认服务已启动'
        return
      }
      const item: SessionItem = {
        id: session_id,
        title: content.slice(0, 40),
        updatedAt: Date.now(),
        messages: [userMsg],
        cursor: undefined,
        isStreaming: true,
      }
      sessions.value.unshift(item)
      saveSessions(sessions.value)
      setLastActiveSessionId(session_id)
      currentSessionId.value = session_id
      messages.value = [userMsg]
      isStreaming.value = true
      try {
        await connectWsForSession(session_id)
        await wsConn.value!.waitOpen()
        wsConn.value!.sendMessage(content)
      } catch (e) {
        markRetryable(session_id, content, e instanceof Error ? e.message : '连接失败，请重试')
      }
      return
    }

    const sid = currentSessionId.value

    const userMsg: ChatMessage = {
      id: genId(),
      role: 'user',
      content,
      timestamp: Date.now(),
    }
    const msgs = cloneMessages(getSessionMessages(sid))
    msgs.push(userMsg)
    commitSessionMessages(sid, msgs)
    setSessionStreaming(sid, true)

    try {
      if (!wsConn.value || wsBoundSessionId.value !== sid) {
        await connectWsForSession(sid)
      }
      await wsConn.value!.waitOpen()
      wsConn.value!.sendMessage(content)
    } catch (e) {
      markRetryable(sid, content, e instanceof Error ? e.message : '发送失败，请重试')
    }
  }

  async function retryQuestion(content: string, userMessageId?: string) {
    if (isStreaming.value || !currentSessionId.value) return

    const sid = currentSessionId.value
    const question = content.trim()
    if (!question) return

    error.value = null
    retryableQuestion.value = null

    let msgs = cloneMessages(getSessionMessages(sid))
    if (userMessageId) {
      const idx = msgs.findIndex((m) => m.id === userMessageId)
      if (idx >= 0) msgs = msgs.slice(0, idx + 1)
    } else {
      msgs = trimAfterLastUser(msgs)
    }
    commitSessionMessages(sid, msgs)
    setSessionStreaming(sid, true)

    try {
      if (!wsConn.value || wsBoundSessionId.value !== sid) {
        await connectWsForSession(sid)
      }
      await wsConn.value!.waitOpen()
      wsConn.value!.sendMessage(question)
    } catch (e) {
      markRetryable(sid, question, e instanceof Error ? e.message : '重试失败，请稍后再试')
    }
  }

  async function retryLastQuestion() {
    if (!retryableQuestion.value) return
    await retryQuestion(retryableQuestion.value)
  }

  async function switchSession(id: string) {
    if (id === currentSessionId.value) return

    // 保存当前会话状态（含进行中的消息）
    saveCurrentSessionMessages()
    const prevIdx = currentSessionId.value ? sessionIndex(currentSessionId.value) : -1
    if (prevIdx !== -1) {
      sessions.value[prevIdx]!.isStreaming = isStreaming.value
    }

    closeWs()

    currentSessionId.value = id
    setLastActiveSessionId(id)
    error.value = null
    retryableQuestion.value = null

    const targetSession = sessions.value.find((s) => s.id === id)
    messages.value = cloneMessages(targetSession?.messages ?? [])
    isStreaming.value = false

    await resumeSession(id)
  }

  function connectWS() {
    void resumeSession()
  }

  async function createNewSession(): Promise<string> {
    saveCurrentSessionMessages()
    let session_id: string
    try {
      ;({ session_id } = await createSession())
    } catch (e) {
      const msg = e instanceof Error ? e.message : '无法创建会话'
      error.value = msg
      throw e
    }
    const item: SessionItem = {
      id: session_id,
      title: '新对话',
      updatedAt: Date.now(),
      messages: [],
      cursor: undefined,
      isStreaming: false,
    }
    sessions.value.unshift(item)
    saveSessions(sessions.value)
    setLastActiveSessionId(session_id)
    return session_id
  }

  async function deleteSession(id: string) {
    console.log('[Store] deleteSession called:', id)
    try {
      const result = await apiDeleteSession(id)
      console.log('[Store] deleteSession result:', result)
    } catch (e) {
      console.error('[Store] Failed to delete session on server:', e)
    }
    const idx = sessionIndex(id)
    if (idx !== -1) {
      sessions.value.splice(idx, 1)
      saveSessions(sessions.value)
      if (currentSessionId.value === id) {
        closeWs()
        currentSessionId.value = sessions.value[0]?.id ?? null
        if (currentSessionId.value) {
          setLastActiveSessionId(currentSessionId.value)
          const targetSession = sessions.value.find((s) => s.id === currentSessionId.value)
          messages.value = cloneMessages(targetSession?.messages ?? [])
          isStreaming.value = Boolean(targetSession?.isStreaming)
          await resumeSession(currentSessionId.value)
        } else {
          messages.value = []
          isStreaming.value = false
          localStorage.removeItem(LAST_ACTIVE_KEY)
        }
      }
    }
  }

  function toggleSidebar() {
    sidebarOpen.value = !sidebarOpen.value
  }

  function stopGeneration() {
    if (!currentSessionId.value) return
    if (wsConn.value) {
      wsConn.value.sendStop()
    }
    const sid = currentSessionId.value
    setSessionStreaming(sid, false)
    const msgs = cloneMessages(getSessionMessages(sid))
    for (const m of msgs) {
      if (m.streaming) {
        m.streaming = false
      }
    }
    commitSessionMessages(sid, msgs)
  }

  function parseDbMessageId(messageId: string): number | null {
    const match = messageId.match(/^(?:done|stream)-(\d+)$/)
    if (!match) return null
    const n = Number(match[1])
    return Number.isNaN(n) ? null : n
  }

  async function submitFeedback(messageId: string, isHelpful: boolean) {
    const sid = currentSessionId.value
    const dbId = parseDbMessageId(messageId)
    if (!sid || dbId == null) return

    const msgs = cloneMessages(getSessionMessages(sid))
    const target = msgs.find((m) => m.id === messageId)
    if (!target || target.feedback) return

    target.feedbackSubmitting = true
    commitSessionMessages(sid, msgs)

    try {
      const result = await apiSubmitFeedback({
        message_id: dbId,
        session_id: sid,
        is_helpful: isHelpful,
      })
      const updated = cloneMessages(getSessionMessages(sid))
      const msg = updated.find((m) => m.id === messageId)
      if (msg) {
        msg.feedback = result.feedback
        msg.feedbackSubmitting = false
      }
      commitSessionMessages(sid, updated)
    } catch (e) {
      console.error('[Store] submitFeedback failed:', e)
      const reverted = cloneMessages(getSessionMessages(sid))
      const msg = reverted.find((m) => m.id === messageId)
      if (msg) msg.feedbackSubmitting = false
      commitSessionMessages(sid, reverted)
      error.value = e instanceof Error ? e.message : '反馈提交失败'
    }
  }

  function clearCurrentMessages() {
    if (currentSessionId.value) {
      const sid = currentSessionId.value
      const idx = sessionIndex(sid)
      if (idx !== -1) {
        sessions.value[idx]!.messages = []
        sessions.value[idx]!.cursor = undefined
        sessions.value[idx]!.isStreaming = false
        saveSessions(sessions.value)
      }
    }
    messages.value = []
    closeWs()
    isStreaming.value = false
    error.value = null
    retryableQuestion.value = null
  }

  function clearError() {
    error.value = null
    retryableQuestion.value = null
  }

  function forceUnstick() {
    closeWs()
    for (const s of sessions.value) {
      s.isStreaming = false
      if (s.messages) {
        for (const m of s.messages) {
          if (m.streaming) m.streaming = false
        }
      }
    }
    saveSessions(sessions.value)
    isStreaming.value = false
    error.value = null
    retryableQuestion.value = null
    if (currentSessionId.value) {
      messages.value = cloneMessages(
        sessions.value.find((s) => s.id === currentSessionId.value)?.messages ?? [],
      )
    }
  }

  return {
    sessions,
    currentSessionId,
    currentSession,
    messages,
    isStreaming,
    streamingStatus,
    sidebarOpen,
    error,
    retryableQuestion,
    canRetry,
    createNewSession,
    switchSession,
    sendMessage,
    retryLastQuestion,
    retryQuestion,
    clearError,
    deleteSession,
    toggleSidebar,
    clearCurrentMessages,
    stopGeneration,
    submitFeedback,
    wsConn,
    connectWS,
    resumeSession,
    forceUnstick,
  }
})
