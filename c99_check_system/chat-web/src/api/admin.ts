import type {
  FeedbackStatsResponse,
  InaccurateFeedbackListResponse,
} from '@/types'

const HTTP_BASE = import.meta.env.VITE_API_BASE ?? 'http://localhost:8000/api/qa'
const ADMIN_TOKEN_KEY = 'qa_admin_token'

export function getAdminToken(): string | null {
  return sessionStorage.getItem(ADMIN_TOKEN_KEY)
}

export function setAdminToken(token: string) {
  sessionStorage.setItem(ADMIN_TOKEN_KEY, token)
}

export function clearAdminToken() {
  sessionStorage.removeItem(ADMIN_TOKEN_KEY)
}

function adminHeaders(): HeadersInit {
  const token = getAdminToken()
  if (!token) throw new Error('未登录')
  return {
    'Content-Type': 'application/json',
    'X-Admin-Session': token,
  }
}

export async function adminLogin(password: string): Promise<{ token: string; expires_in: number }> {
  const res = await fetch(`${HTTP_BASE}/admin/login`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ password }),
  })
  if (!res.ok) {
    const err = await res.json().catch(() => ({}))
    throw new Error((err as { detail?: string }).detail ?? '登录失败')
  }
  const data = await res.json()
  setAdminToken(data.token)
  return data
}

export async function adminLogout() {
  const token = getAdminToken()
  if (token) {
    await fetch(`${HTTP_BASE}/admin/logout`, {
      method: 'POST',
      headers: { 'X-Admin-Session': token },
    }).catch(() => {})
  }
  clearAdminToken()
}

export async function fetchFeedbackStats(days = 30, topN = 20): Promise<FeedbackStatsResponse> {
  const res = await fetch(
    `${HTTP_BASE}/admin/feedback/stats?days=${days}&top_n=${topN}`,
    { headers: adminHeaders() },
  )
  if (res.status === 401) {
    clearAdminToken()
    throw new Error('登录已过期，请重新登录')
  }
  if (!res.ok) throw new Error(`统计加载失败: ${res.status}`)
  return res.json()
}

export async function fetchInaccurateList(
  limit = 50,
  offset = 0,
): Promise<InaccurateFeedbackListResponse> {
  const res = await fetch(
    `${HTTP_BASE}/admin/feedback/inaccurate?limit=${limit}&offset=${offset}`,
    { headers: adminHeaders() },
  )
  if (res.status === 401) {
    clearAdminToken()
    throw new Error('登录已过期，请重新登录')
  }
  if (!res.ok) throw new Error(`列表加载失败: ${res.status}`)
  return res.json()
}
