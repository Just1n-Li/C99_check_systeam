<script setup lang="ts">
import { onMounted, ref } from 'vue'
import { useRouter } from 'vue-router'
import {
  adminLogout,
  fetchFeedbackStats,
  fetchInaccurateList,
  getAdminToken,
} from '@/api/admin'
import type { FeedbackStatsResponse, InaccurateFeedbackItem } from '@/types'

const router = useRouter()
const loading = ref(true)
const error = ref('')
const stats = ref<FeedbackStatsResponse | null>(null)
const inaccurateItems = ref<InaccurateFeedbackItem[]>([])
const inaccurateTotal = ref(0)
const days = ref(30)

async function loadData() {
  if (!getAdminToken()) {
    await router.replace('/admin/login')
    return
  }
  loading.value = true
  error.value = ''
  try {
    const [statsData, listData] = await Promise.all([
      fetchFeedbackStats(days.value, 15),
      fetchInaccurateList(30, 0),
    ])
    stats.value = statsData
    inaccurateItems.value = listData.items
    inaccurateTotal.value = listData.total
  } catch (e) {
    error.value = e instanceof Error ? e.message : '加载失败'
    if (error.value.includes('过期') || error.value.includes('未登录')) {
      await router.replace('/admin/login')
    }
  } finally {
    loading.value = false
  }
}

async function handleLogout() {
  await adminLogout()
  await router.replace('/admin/login')
}

function formatRate(rate: number) {
  return `${(rate * 100).toFixed(1)}%`
}

function formatTime(iso?: string | null) {
  if (!iso) return '—'
  return new Date(iso).toLocaleString('zh-CN')
}

onMounted(() => { void loadData() })
</script>

<template>
  <div class="admin-page">
    <header class="admin-header">
      <div>
        <h1 class="admin-title">反馈管理</h1>
        <p class="admin-sub">问答质量统计 · 不准确回答审核</p>
      </div>
      <div class="admin-header-actions">
        <router-link to="/" class="header-link">返回问答</router-link>
        <button type="button" class="header-btn" @click="handleLogout">退出登录</button>
      </div>
    </header>

    <main class="admin-main">
      <div v-if="loading" class="admin-loading">加载中…</div>
      <p v-else-if="error" class="admin-error">{{ error }}</p>

      <template v-else-if="stats">
        <section class="stats-grid">
          <div class="stat-card">
            <span class="stat-label">总反馈数</span>
            <span class="stat-value">{{ stats.summary.total_feedback }}</span>
          </div>
          <div class="stat-card stat-card--ok">
            <span class="stat-label">有帮助</span>
            <span class="stat-value">{{ stats.summary.helpful_count }}</span>
          </div>
          <div class="stat-card stat-card--bad">
            <span class="stat-label">不准确</span>
            <span class="stat-value">{{ stats.summary.inaccurate_count }}</span>
          </div>
          <div class="stat-card">
            <span class="stat-label">不准确率（近 {{ stats.days }} 天）</span>
            <span class="stat-value">{{ formatRate(stats.summary.inaccurate_rate) }}</span>
          </div>
        </section>

        <section class="panel">
          <h2 class="panel-title">按意图统计（不准确次数）</h2>
          <p v-if="stats.by_intent.length === 0" class="panel-empty">暂无数据</p>
          <table v-else class="data-table">
            <thead>
              <tr>
                <th>意图类型</th>
                <th>不准确</th>
                <th>总反馈</th>
              </tr>
            </thead>
            <tbody>
              <tr v-for="row in stats.by_intent" :key="row.intent">
                <td>{{ row.intent }}</td>
                <td class="text-bad">{{ row.inaccurate_count }}</td>
                <td>{{ row.total_count }}</td>
              </tr>
            </tbody>
          </table>
        </section>

        <section class="panel">
          <h2 class="panel-title">高频错误问题 Top {{ stats.top_inaccurate_questions.length }}</h2>
          <p v-if="stats.top_inaccurate_questions.length === 0" class="panel-empty">暂无被标记为不准确的问题</p>
          <table v-else class="data-table">
            <thead>
              <tr>
                <th>问题</th>
                <th>次数</th>
                <th>意图</th>
                <th>最近反馈</th>
              </tr>
            </thead>
            <tbody>
              <tr v-for="(row, i) in stats.top_inaccurate_questions" :key="i">
                <td class="col-question">{{ row.question }}</td>
                <td class="text-bad">{{ row.count }}</td>
                <td>{{ row.intent ?? '—' }}</td>
                <td class="col-time">{{ formatTime(row.latest_at) }}</td>
              </tr>
            </tbody>
          </table>
        </section>

        <section class="panel">
          <h2 class="panel-title">不准确回答审核列表（{{ inaccurateTotal }} 条）</h2>
          <p class="panel-hint">供人工核对：用户原问题 + 系统回答</p>
          <p v-if="inaccurateItems.length === 0" class="panel-empty">暂无待审核记录</p>
          <div v-else class="review-list">
            <article v-for="item in inaccurateItems" :key="item.feedback_id" class="review-item">
              <div class="review-meta">
                <span>#{{ item.feedback_id }}</span>
                <span>{{ formatTime(item.created_at) }}</span>
                <span v-if="item.intent">{{ item.intent }}</span>
              </div>
              <p class="review-q"><strong>问：</strong>{{ item.question || '（无记录）' }}</p>
              <p class="review-a"><strong>答：</strong>{{ item.answer || '（无记录）' }}</p>
            </article>
          </div>
        </section>
      </template>
    </main>
  </div>
</template>

<style scoped>
.admin-page {
  height: 100%;
  min-height: 0;
  overflow-y: auto;
  overflow-x: hidden;
  background: var(--color-bg);
  -webkit-overflow-scrolling: touch;
}
.admin-header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  gap: 16px;
  padding: 20px 28px;
  background: #161b22;
  border-bottom: 1px solid var(--color-border);
}
.admin-title {
  font-size: 20px;
  font-weight: 700;
  color: var(--color-text);
}
.admin-sub {
  font-size: 13px;
  color: var(--color-text-muted);
  margin-top: 2px;
}
.admin-header-actions {
  display: flex;
  align-items: center;
  gap: 12px;
}
.header-link {
  font-size: 13px;
  color: var(--color-text-muted);
  text-decoration: none;
}
.header-link:hover { color: var(--color-accent); }
.header-btn {
  font-size: 13px;
  padding: 7px 12px;
  border-radius: 8px;
  border: 1px solid var(--color-border);
  color: var(--color-text-muted);
  transition: background 0.15s;
}
.header-btn:hover { background: var(--color-surface); }

.admin-main {
  max-width: 960px;
  margin: 0 auto;
  padding: 24px 28px 40px;
}
.admin-loading, .admin-error {
  font-size: 14px;
  color: var(--color-text-muted);
  padding: 24px 0;
}
.admin-error { color: #ef4444; }

.stats-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(180px, 1fr));
  gap: 12px;
  margin-bottom: 24px;
}
.stat-card {
  background: #161b22;
  border: 1px solid var(--color-border);
  border-radius: 12px;
  padding: 16px 18px;
}
.stat-card--ok { border-color: rgba(34,211,238,0.35); background: rgba(34,211,238,0.12); }
.stat-card--bad { border-color: #fecaca; background: #2a1619; }
.stat-label {
  display: block;
  font-size: 12px;
  color: var(--color-text-muted);
  margin-bottom: 6px;
}
.stat-value {
  font-size: 26px;
  font-weight: 700;
  color: var(--color-text);
}
.stat-card--ok .stat-value { color: var(--color-accent); }
.stat-card--bad .stat-value { color: #ef4444; }

.panel {
  background: #161b22;
  border: 1px solid var(--color-border);
  border-radius: 12px;
  padding: 18px 20px;
  margin-bottom: 20px;
  overflow-x: auto;
}
.panel-title {
  font-size: 15px;
  font-weight: 600;
  margin-bottom: 12px;
}
.panel-hint {
  font-size: 12px;
  color: var(--color-text-subtle);
  margin: -6px 0 12px;
}
.panel-empty {
  font-size: 13px;
  color: var(--color-text-subtle);
  padding: 8px 0;
}

.data-table {
  width: 100%;
  border-collapse: collapse;
  font-size: 13px;
}
.data-table th {
  text-align: left;
  padding: 8px 10px;
  background: #141a21;
  border-bottom: 1px solid var(--color-border);
  color: var(--color-text-muted);
  font-weight: 500;
}
.data-table td {
  padding: 10px;
  border-bottom: 1px solid #f4f4f5;
  vertical-align: top;
}
.col-question { max-width: 360px; word-break: break-word; }
.col-time { white-space: nowrap; color: var(--color-text-subtle); font-size: 12px; }
.text-bad { color: #ef4444; font-weight: 600; }

.review-list {
  display: flex;
  flex-direction: column;
  gap: 12px;
}
.review-item {
  border: 1px solid #f4f4f5;
  border-radius: 10px;
  padding: 12px 14px;
  background: #141a21;
}
.review-meta {
  display: flex;
  flex-wrap: wrap;
  gap: 10px;
  font-size: 11px;
  color: var(--color-text-subtle);
  margin-bottom: 8px;
}
.review-q, .review-a {
  font-size: 13px;
  line-height: 1.6;
  color: var(--color-text);
  margin-top: 6px;
  word-break: break-word;
}
.review-a { color: var(--color-text-muted); }
</style>
