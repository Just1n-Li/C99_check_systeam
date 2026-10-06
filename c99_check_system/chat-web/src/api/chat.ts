import type { WSServerMessage, FeedbackType } from '@/types'

export const HTTP_BASE = import.meta.env.VITE_API_BASE ?? 'http://localhost:8000/api/qa'
const WS_BASE = (import.meta.env.VITE_WS_BASE as string | undefined)
  ?? `${HTTP_BASE.replace(/^http/, 'ws')}/ws`

const FETCH_TIMEOUT_MS = 8_000

async function fetchWithTimeout(url: string, options: RequestInit = {}): Promise<Response> {
  const controller = new AbortController()
  const timer = setTimeout(() => controller.abort(), FETCH_TIMEOUT_MS)
  try {
    return await fetch(url, { ...options, signal: controller.signal })
  } catch (e) {
    if (e instanceof DOMException && e.name === 'AbortError') {
      throw new Error(`请求超时（${FETCH_TIMEOUT_MS / 1000}s），请检查后端是否可达：${HTTP_BASE}`)
    }
    throw new Error(`无法连接后端：${HTTP_BASE}`)
  } finally {
    clearTimeout(timer)
  }
}

export async function createSession(): Promise<{ session_id: string }> {
  const res = await fetchWithTimeout(`${HTTP_BASE}/session`, { method: 'POST' })
  if (!res.ok) throw new Error(`Session creation failed: ${res.status}`)
  return res.json()
}

export async function deleteSession(sessionId: string): Promise<{ session_id: string; deleted_count: number }> {
  console.log('[API] deleteSession called:', sessionId)
  const url = `${HTTP_BASE}/session/${sessionId}`
  console.log('[API] DELETE URL:', url)
  const res = await fetchWithTimeout(url, { method: 'DELETE' })
  console.log('[API] deleteSession response status:', res.status)
  if (!res.ok) throw new Error(`Session deletion failed: ${res.status}`)
  const result = await res.json()
  console.log('[API] deleteSession result:', result)
  return result
}

export async function fetchHistory(sessionId: string) {
  const res = await fetchWithTimeout(`${HTTP_BASE}/history/${sessionId}`)
  if (!res.ok) throw new Error(`History fetch failed: ${res.status}`)
  return res.json()
}

export interface FeedbackPayload {
  message_id: number
  session_id?: string
  is_helpful: boolean
}

export interface FeedbackResponse {
  status: string
  message: string
  message_id: number
  is_helpful: boolean
  feedback: FeedbackType
}

export async function submitFeedback(payload: FeedbackPayload): Promise<FeedbackResponse> {
  const res = await fetchWithTimeout(`${HTTP_BASE}/feedback`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(payload),
  })
  if (!res.ok) {
    const err = await res.json().catch(() => ({}))
    throw new Error((err as { detail?: string }).detail ?? `Feedback failed: ${res.status}`)
  }
  return res.json()
}

type MessageHandler = (msg: WSServerMessage) => void
type CloseHandler = () => void

class WSConnection {
  private ws: WebSocket
  private _onMessage: MessageHandler
  private _onClose: CloseHandler
  private _openPromise: Promise<void>
  private _resolved = false

  constructor(
    sessionId: string,
    onMessage: MessageHandler,
    onClose: CloseHandler,
    cursor?: string,
  ) {
    this._onMessage = onMessage
    this._onClose = onClose
    const params = new URLSearchParams({ session_id: sessionId })
    if (cursor) {
      params.set('cursor', cursor)
    }
    this.ws = new WebSocket(`${WS_BASE}?${params.toString()}`)

    this._openPromise = new Promise((resolve, reject) => {
      this.ws.onopen = () => {
        this._resolved = true
        console.log('[WS] Connected')
        resolve()
      }
      this.ws.onerror = (e) => {
        console.error('[WS] Error:', e)
        if (!this._resolved) {
          this._resolved = true
          reject(new Error('WebSocket connection error'))
        }
        this._onClose()
      }
    })

    this.ws.onmessage = (evt) => {
      try {
        const data = JSON.parse(evt.data) as WSServerMessage
        this._onMessage(data)
      } catch {
        this._onMessage({ type: 'error', error: 'Invalid JSON from server' })
      }
    }

    this.ws.onclose = () => this._onClose()
  }

  async waitOpen(): Promise<void> {
    if (this.ws.readyState === WebSocket.OPEN) return
    return this._openPromise
  }

  send(data: object) {
    if (this.ws.readyState === WebSocket.OPEN) {
      this.ws.send(JSON.stringify(data))
    }
  }

  sendMessage(content: string) {
    this.send({ type: 'message', content })
  }

  sendResume(cursor: string) {
    this.send({ type: 'resume', cursor })
  }

  sendPing() {
    this.send({ type: 'ping' })
  }

  sendStop() {
    this.send({ type: 'stop' })
  }

  close() {
    this.ws.close()
  }
}

export { WSConnection }