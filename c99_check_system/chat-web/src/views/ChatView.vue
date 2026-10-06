<script setup lang="ts">
import { onMounted, onUnmounted } from 'vue'
import { useChatStore } from '@/stores/chat'
import Sidebar from '@/components/Sidebar.vue'
import MessageList from '@/components/MessageList.vue'
import ChatInput from '@/components/ChatInput.vue'

const store = useChatStore()

function onVisibilityChange() {
  if (document.visibilityState === 'visible' && store.currentSessionId) {
    const session = store.sessions.find((s) => s.id === store.currentSessionId)
    if (session?.isStreaming || store.isStreaming) {
      void store.resumeSession(store.currentSessionId)
    }
  }
}

onMounted(() => {
  if (store.currentSessionId) {
    void store.resumeSession()
  }
  document.addEventListener('visibilitychange', onVisibilityChange)
})

onUnmounted(() => {
  document.removeEventListener('visibilitychange', onVisibilityChange)
})
</script>

<template>
  <div class="app-root">
    <div v-if="store.error" class="app-alert">
      <span class="app-alert-text">{{ store.error }}</span>
      <div class="app-alert-actions">
        <button
          type="button"
          class="app-alert-btn app-alert-btn--ghost"
          @click="store.forceUnstick()"
        >
          解除卡住
        </button>
        <button
          type="button"
          class="app-alert-btn app-alert-btn--ghost"
          @click="store.clearError()"
        >
          关闭
        </button>
      </div>
    </div>
    <div class="app-body-row">
    <Sidebar />
    <div class="app-main">
      <header class="app-header">
        <button class="icon-btn" aria-label="切换侧边栏" @click="store.toggleSidebar">
          <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
            <path d="M3 12h18M3 6h18M3 18h18"/>
          </svg>
        </button>
        <div class="header-center">
          <span class="header-brand">C99 编译器测试</span>
          <span class="header-sub">条款 → 测试代码生成</span>
        </div>
      </header>

      <div v-if="store.messages.length > 0" class="app-body">
        <MessageList />
      </div>

      <div v-else class="app-center">
        <div class="center-col">
          <div class="center-icon" aria-hidden="true">
            <svg width="32" height="32" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5">
              <path d="M3 9h18v10a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2V9z"/>
              <path d="M3 9l2.5-5h13L21 9"/>
              <path d="M8 14h.01M12 14h.01M16 14h.01"/>
            </svg>
          </div>
          <h1 class="center-title">输入 C99 条款，生成编译器符合性测试代码</h1>
          <p class="center-sub">基于 C99 标准语义检索，自动生成用于验证编译器是否符合条款的 C 测试程序</p>
          <div class="center-suggestions">
            <button class="suggestion-chip" @click="store.sendMessage('6.5.2.2')">
              函数调用（编号）
            </button>
            <button class="suggestion-chip" @click="store.sendMessage('函数调用的规则')">
              函数调用（描述）
            </button>
            <button class="suggestion-chip" @click="store.sendMessage('6.5.6')">
              加法运算符（编号）
            </button>
          </div>
          <ChatInput />
        </div>
      </div>

      <ChatInput v-if="store.messages.length > 0" />
    </div>
    </div>
  </div>
</template>

<style scoped>
.app-root {
  display: flex;
  flex-direction: column;
  height: 100%;
  background: var(--color-bg);
}
.app-alert {
  display: flex;
  align-items: center;
  justify-content: space-between;
  gap: 12px;
  padding: 10px 16px;
  background: #2a1619;
  border-bottom: 1px solid #3a1d21;
  color: #f87171;
  font-size: 13px;
  flex-shrink: 0;
  z-index: 20;
}
.app-alert-text {
  flex: 1;
  min-width: 0;
  line-height: 1.4;
}
.app-alert-actions {
  display: flex;
  gap: 8px;
  flex-shrink: 0;
}
.app-alert-btn {
  font-size: 12px;
  padding: 5px 10px;
  border-radius: 6px;
  background: #ef4444;
  color: #fff;
  white-space: nowrap;
}
.app-alert-btn--ghost {
  background: transparent;
  color: #f87171;
  border: 1px solid #3a1d21;
}
.app-body-row {
  display: flex;
  flex: 1;
  min-height: 0;
  overflow: hidden;
}
.app-main {
  flex: 1;
  display: flex;
  flex-direction: column;
  min-width: 0;
  height: 100%;
  position: relative;
  background:
    radial-gradient(ellipse 80% 50% at 50% -20%, rgba(5, 150, 105, 0.06), transparent),
    var(--color-bg);
}
.app-header {
  display: flex;
  align-items: center;
  justify-content: center;
  height: 52px;
  padding: 0 16px;
  flex-shrink: 0;
  background: rgba(11, 15, 20, 0.82);
  backdrop-filter: blur(12px);
  border-bottom: 1px solid var(--color-border);
  position: relative;
  z-index: 10;
}
.icon-btn {
  position: absolute;
  left: 16px;
  display: flex;
  align-items: center;
  justify-content: center;
  width: 34px;
  height: 34px;
  border-radius: 10px;
  color: var(--color-text-subtle);
  transition: color 0.15s, background 0.15s;
}
.icon-btn:hover {
  color: var(--color-text);
  background: var(--color-surface);
}
.header-center {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 1px;
}
.header-brand {
  font-size: 15px;
  font-weight: 600;
  color: var(--color-text);
  letter-spacing: -0.01em;
}
.header-sub {
  font-size: 11px;
  color: var(--color-text-subtle);
  letter-spacing: 0.02em;
}
.app-body {
  flex: 1;
  display: flex;
  flex-direction: column;
  min-height: 0;
  overflow: hidden;
}
.app-center {
  flex: 1;
  display: flex;
  align-items: center;
  justify-content: center;
  padding: 24px 0;
}
.center-col {
  display: flex;
  flex-direction: column;
  align-items: center;
  width: 100%;
  max-width: 680px;
  padding: 0 24px;
}
.center-icon {
  width: 64px;
  height: 64px;
  border-radius: 18px;
  display: flex;
  align-items: center;
  justify-content: center;
  background: linear-gradient(135deg, rgba(34,211,238,0.12) 0%, rgba(34,211,238,0.2) 100%);
  color: var(--color-accent);
  border: 1px solid rgba(34,211,238,0.35);
  margin-bottom: 20px;
  box-shadow: 0 4px 20px rgba(5, 150, 105, 0.12);
}
.center-title {
  font-size: 26px;
  font-weight: 700;
  color: var(--color-text);
  letter-spacing: -0.025em;
  margin-bottom: 10px;
  line-height: 1.25;
  text-align: center;
}
.center-sub {
  font-size: 14px;
  color: var(--color-text-muted);
  margin-bottom: 24px;
  text-align: center;
  line-height: 1.6;
  max-width: 420px;
}
.center-suggestions {
  display: flex;
  flex-wrap: wrap;
  justify-content: center;
  gap: 8px;
  margin-bottom: 28px;
}
.suggestion-chip {
  font-size: 13px;
  color: var(--color-text-muted);
  background: #161b22;
  border: 1px solid var(--color-border);
  border-radius: 999px;
  padding: 8px 14px;
  transition: color 0.15s, border-color 0.15s, background 0.15s, transform 0.1s;
}
.suggestion-chip:hover {
  color: var(--color-accent);
  border-color: rgba(34,211,238,0.35);
  background: rgba(34,211,238,0.12);
}
.suggestion-chip:active {
  transform: scale(0.97);
}

@media (max-width: 640px) {
  .center-title { font-size: 22px; }
  .center-suggestions { gap: 6px; }
  .suggestion-chip { font-size: 12px; padding: 7px 12px; }
}
</style>
