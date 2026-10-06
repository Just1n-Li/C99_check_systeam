<script setup lang="ts">
import { useChatStore } from '@/stores/chat'

const store = useChatStore()

function formatTime(ts: number): string {
  const diff = Math.floor((Date.now() - ts) / 60000)
  if (diff < 1) return '刚刚'
  if (diff < 60) return `${diff}分钟前`
  if (diff < 1440) return `${Math.floor(diff / 60)}小时前`
  return new Date(ts).toLocaleDateString('zh-CN', { month: 'short', day: 'numeric' })
}

async function handleNewSession() {
  try {
    const id = await store.createNewSession()
    await store.switchSession(id)
  } catch {
    // error 已在 store 中设置
  }
}
</script>

<template>
  <aside class="sidebar" :class="{ 'sidebar--open': store.sidebarOpen }">
    <div class="sidebar-inner">
      <div class="sidebar-header">
        <span class="sidebar-label">历史记录</span>
        <button
          class="sidebar-add-btn"
          title="新建对话"
          @click="handleNewSession"
        >
          <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5">
            <path d="M12 5v14M5 12h14"/>
          </svg>
        </button>
      </div>

      <nav class="sidebar-nav">
        <div v-if="store.sessions.length === 0" class="sidebar-empty">
          <svg width="22" height="22" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5">
            <path d="M21 15a2 2 0 0 1-2 2H7l-4 4V5a2 2 0 0 1 2-2h14a2 2 0 0 1 2 2z"/>
          </svg>
          <p>暂无对话记录</p>
        </div>

        <div
          v-for="session in store.sessions"
          :key="session.id"
          class="sidebar-item"
          :class="{ 'sidebar-item--active': store.currentSessionId === session.id }"
          @click="store.switchSession(session.id)"
        >
          <div class="sidebar-item-dot" :class="{ 'dot--active': store.currentSessionId === session.id }" />
          <div class="sidebar-item-body">
            <p class="sidebar-item-title">{{ session.title || '新对话' }}</p>
            <p class="sidebar-item-time">{{ formatTime(session.updatedAt) }}</p>
          </div>
          <button
            class="sidebar-item-del"
            title="删除"
            @click.stop="store.deleteSession(session.id)"
          >
            <svg width="11" height="11" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
              <path d="M18 6L6 18M6 6l12 12"/>
            </svg>
          </button>
        </div>
      </nav>

      <div class="sidebar-footer">
        <p>C99 编译器测试</p>
        <router-link to="/admin/login" class="sidebar-admin-link">管理入口</router-link>
      </div>
    </div>
  </aside>
</template>

<style scoped>
.sidebar {
  width: 0;
  flex-shrink: 0;
  height: 100%;
  overflow: hidden;
  transition: width 0.3s cubic-bezier(0.16, 1, 0.3, 1);
  background: var(--color-sidebar-bg);
  border-right: 1px solid var(--color-border);
  pointer-events: none;
}
.sidebar--open {
  width: 260px;
  pointer-events: auto;
}
.sidebar-inner {
  width: 260px;
  height: 100%;
  display: flex;
  flex-direction: column;
}
.sidebar-header {
  display: flex;
  align-items: center;
  height: 48px;
  padding: 0 12px;
  border-bottom: 1px solid var(--color-border);
  gap: 8px;
  flex-shrink: 0;
}
.sidebar-label {
  flex: 1;
  font-size: 13px;
  font-weight: 500;
  color: #6b7887;
}
.sidebar-add-btn {
  display: flex;
  align-items: center;
  justify-content: center;
  width: 28px;
  height: 28px;
  border-radius: 8px;
  color: #6b7887;
  transition: color 0.15s, background 0.15s;
}
.sidebar-add-btn:hover {
  color: var(--color-accent);
  background: var(--color-accent-light);
}
.sidebar-nav {
  flex: 1;
  overflow-y: auto;
  padding: 8px 0;
}
.sidebar-empty {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 8px;
  padding: 32px 16px;
  color: #3b4756;
}
.sidebar-empty p {
  font-size: 12px;
}
.sidebar-item {
  display: flex;
  align-items: center;
  gap: 10px;
  margin: 2px 8px;
  padding: 10px 10px;
  border-radius: 10px;
  border: 1px solid transparent;
  cursor: pointer;
  transition: background 0.15s, border-color 0.15s;
  position: relative;
}
.sidebar-item:hover {
  background: #1a212b;
}
.sidebar-item--active {
  background: rgba(34,211,238,0.12);
  border: 1px solid rgba(34,211,238,0.35);
}
.sidebar-item-dot {
  width: 6px;
  height: 6px;
  border-radius: 50%;
  background: #3b4756;
  flex-shrink: 0;
  transition: background 0.15s;
}
.dot--active {
  background: var(--color-accent);
}
.sidebar-item-body {
  flex: 1;
  min-width: 0;
}
.sidebar-item-title {
  font-size: 13px;
  font-weight: 500;
  color: #9aa7b4;
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
  line-height: 1.4;
}
.sidebar-item--active .sidebar-item-title {
  color: #e6edf3;
}
.sidebar-item-time {
  font-size: 11px;
  color: #6b7887;
  margin-top: 2px;
}
.sidebar-item-del {
  position: absolute;
  right: 8px;
  top: 50%;
  transform: translateY(-50%);
  width: 22px;
  height: 22px;
  border-radius: 6px;
  display: flex;
  align-items: center;
  justify-content: center;
  color: #6b7887;
  opacity: 0;
  transition: opacity 0.15s, color 0.15s, background 0.15s;
}
.sidebar-item:hover .sidebar-item-del {
  opacity: 1;
}
.sidebar-item-del:hover {
  color: #ef4444;
  background: #2a1619;
}
.sidebar-footer {
  padding: 10px 16px;
  border-top: 1px solid var(--color-border);
  flex-shrink: 0;
}
.sidebar-footer p {
  font-size: 11px;
  color: #3b4756;
}
.sidebar-admin-link {
  display: inline-block;
  margin-top: 6px;
  font-size: 11px;
  color: var(--color-text-subtle);
  text-decoration: none;
}
.sidebar-admin-link:hover {
  color: var(--color-accent);
}
</style>