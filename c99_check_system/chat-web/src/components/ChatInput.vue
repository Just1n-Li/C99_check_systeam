<script setup lang="ts">
import { ref, computed } from 'vue'
import { useChatStore } from '@/stores/chat'

const store = useChatStore()
const inputValue = ref('')

function handleSubmit() {
  const text = inputValue.value.trim()
  if (!text || store.isStreaming) return
  inputValue.value = ''
  store.sendMessage(text)
}

function handleKeydown(e: KeyboardEvent) {
  if (e.key === 'Enter' && !e.shiftKey) {
    e.preventDefault()
    handleSubmit()
  }
}

function stopGeneration() {
  store.stopGeneration()
}

// ── 状态条：根据当前 phase 选 SVG 图标 ─────────────────────────────
const SPINNER_SVG = `<svg viewBox="0 0 50 50" width="14" height="14"><circle cx="25" cy="25" r="20" fill="none" stroke="currentColor" stroke-width="4" stroke-dasharray="80 200" stroke-linecap="round"/></svg>`

const RAG_ICON_SVG = `<svg viewBox="0 0 24 24" width="14" height="14" fill="none" stroke="currentColor" stroke-width="1.75" stroke-linecap="round" stroke-linejoin="round"><path d="M12 2L2 7l10 5 10-5-10-5z"/><path d="M2 17l10 5 10-5"/><path d="M2 12l10 5 10-5"/></svg>`

const MAIN_ICON_SVG = `<svg viewBox="0 0 24 24" width="14" height="14" fill="none" stroke="currentColor" stroke-width="1.75" stroke-linecap="round" stroke-linejoin="round"><path d="M12 2a7 7 0 0 0-4 12.7c.6.5 1 1.2 1 2v.3h6v-.3c0-.8.4-1.5 1-2A7 7 0 0 0 12 2z"/><path d="M9 18h6"/><path d="M10 22h4"/></svg>`

const statusIcon = computed(() => {
  const s = store.streamingStatus
  if (!s) return SPINNER_SVG
  if (s.phase === 'rag') return RAG_ICON_SVG
  if (s.phase === 'main') return MAIN_ICON_SVG
  return SPINNER_SVG
})
</script>

<template>
  <div class="input-wrap">
    <transition name="status-fade">
      <div
        v-if="store.streamingStatus"
        class="status-bar"
        :class="`status-bar--${store.streamingStatus.phase}`"
      >
        <span class="status-icon status-spin" v-html="statusIcon" />
        <span class="status-text">{{ store.streamingStatus.label }}</span>
        <span v-if="store.streamingStatus.detail" class="status-detail">
          {{ store.streamingStatus.detail }}
        </span>
        <button
          class="status-cancel"
          title="停止生成"
          @click="stopGeneration"
        >
          <svg viewBox="0 0 24 24" width="11" height="11" fill="none" stroke="currentColor" stroke-width="2.5" stroke-linecap="round">
            <line x1="6" y1="6" x2="18" y2="18"/>
            <line x1="6" y1="18" x2="18" y2="6"/>
          </svg>
        </button>
      </div>
    </transition>

    <div class="input-inner">
      <textarea
        v-model="inputValue"
        class="input-field"
        placeholder="输入 C99 条款编号（如 6.5.2.2）或中文描述（如「函数调用的规则」）"
        rows="1"
        @keydown="handleKeydown"
      />
      <button
        class="send-btn"
        :class="{
          'send-btn--active': inputValue.trim() && !store.isStreaming,
          'send-btn--stop': store.isStreaming,
        }"
        :disabled="!inputValue.trim() && !store.isStreaming"
        :title="store.isStreaming ? '停止生成' : '发送'"
        @click="store.isStreaming ? stopGeneration() : handleSubmit()"
      >
        <svg v-if="store.isStreaming" width="10" height="10" viewBox="0 0 24 24" fill="currentColor">
          <rect x="6" y="6" width="12" height="12" rx="1"/>
        </svg>
        <svg v-else width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
          <path d="M22 2L11 13M22 2L15 22l-4-9-9-4 20-7z"/>
        </svg>
      </button>
    </div>
  </div>
</template>

<style scoped>
.input-wrap {
  padding: 0 24px 20px;
  flex-shrink: 0;
  width: 100%;
}

/* ── 状态条 ─────────────────────────────────────────────────────── */
.status-bar {
  display: flex;
  align-items: center;
  gap: 9px;
  padding: 8px 14px;
  margin-bottom: 10px;
  border-radius: 10px;
  font-size: 12.5px;
  line-height: 1.4;
  border: 1px solid;
  backdrop-filter: blur(6px);
}
.status-bar--connecting {
  background: #1a212b;
  border-color: #232b36;
  color: #9aa7b4;
}
.status-bar--rag {
  background: linear-gradient(to right, rgba(167,139,250,0.08) 0%, rgba(167,139,250,0.15) 100%);
  border-color: rgba(167,139,250,0.3);
  color: #c4b5fd;
}
.status-bar--main {
  background: linear-gradient(to right, rgba(59,130,246,0.08) 0%, rgba(59,130,246,0.15) 100%);
  border-color: rgba(59,130,246,0.3);
  color: #60a5fa;
}
.status-icon {
  display: inline-flex;
  align-items: center;
  justify-content: center;
  flex-shrink: 0;
  width: 16px;
  height: 16px;
}
.status-spin {
  animation: status-spin-anim 0.9s linear infinite;
  transform-origin: center;
}
@keyframes status-spin-anim {
  from { transform: rotate(0deg); }
  to   { transform: rotate(360deg); }
}
.status-text {
  font-weight: 600;
  white-space: nowrap;
  flex-shrink: 0;
}
.status-detail {
  color: inherit;
  opacity: 0.72;
  font-weight: 400;
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
  min-width: 0;
  flex: 1;
}
.status-cancel {
  display: inline-flex;
  align-items: center;
  justify-content: center;
  width: 20px;
  height: 20px;
  border-radius: 4px;
  border: none;
  background: transparent;
  color: inherit;
  opacity: 0.55;
  cursor: pointer;
  transition: opacity 0.15s, background 0.15s;
  flex-shrink: 0;
  padding: 0;
}
.status-cancel:hover {
  opacity: 1;
  background: rgba(0, 0, 0, 0.08);
}
.status-fade-enter-active,
.status-fade-leave-active {
  transition: opacity 0.22s ease, transform 0.22s ease;
}
.status-fade-enter-from,
.status-fade-leave-to {
  opacity: 0;
  transform: translateY(-4px);
}

/* ── 输入框 ─────────────────────────────────────────────────────── */
.input-inner {
  display: flex;
  align-items: flex-end;
  gap: 10px;
  width: 100%;
  background: #161b22;
  border: 1px solid var(--color-border);
  border-radius: var(--radius-input);
  box-shadow: var(--shadow-input);
  padding: 14px 16px 14px 20px;
  transition: border-color 0.2s, box-shadow 0.2s;
}
.input-inner:focus-within {
  border-color: rgba(34,211,238,0.35);
  box-shadow: 0 4px 32px rgba(5, 150, 105, 0.1);
}

.input-field {
  flex: 1;
  background: transparent;
  border: none;
  outline: none;
  resize: none;
  font-family: var(--font-sans);
  font-size: 16px;
  line-height: 26px;
  color: #e6edf3;
  min-height: 26px;
  max-height: 160px;
  overflow-y: auto;
  field-sizing: content;
  caret-color: var(--color-accent);
}
.input-field::placeholder {
  color: #6b7887;
}
.send-btn {
  width: 36px;
  height: 36px;
  border-radius: 50%;
  display: flex;
  align-items: center;
  justify-content: center;
  flex-shrink: 0;
  transition: background 0.15s, transform 0.1s;
  background: #1a212b;
  color: #6b7887;
}
.send-btn:active {
  transform: scale(0.92);
}
.send-btn--active {
  background: var(--color-accent);
  color: #161b22;
}
.send-btn--active:hover {
  background: var(--color-accent-hover);
}
.send-btn--stop {
  background: #2a1619;
  color: #ef4444;
}
.send-btn--stop:hover {
  background: #3a1d21;
}
</style>
