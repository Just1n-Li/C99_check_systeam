<script setup lang="ts">
import { nextTick, ref, watch } from 'vue'
import { useChatStore } from '@/stores/chat'
import MessageItem from './MessageItem.vue'

const store = useChatStore()
const listEl = ref<HTMLElement | null>(null)

async function scrollToBottom() {
  await nextTick()
  if (listEl.value) {
    listEl.value.scrollTop = listEl.value.scrollHeight
  }
}

watch(() => store.messages.length, () => scrollToBottom(), { deep: true })
watch(() => store.isStreaming, (v) => { if (v) nextTick(() => scrollToBottom()) })
</script>

<template>
  <div class="msg-list">
    <div class="msg-scroll" ref="listEl">
      <div class="msg-content">
        <MessageItem
          v-for="msg in store.messages"
          :key="msg.id"
          :message="msg"
        />
        <div v-if="store.error || store.canRetry" class="msg-error">
          <div class="msg-error-body">
            <span v-if="store.error">{{ store.error }}</span>
            <span v-else>上次提问未成功完成，可点击消息旁的 ↻ 重新提问</span>
          </div>
          <button class="msg-error-dismiss" aria-label="关闭错误提示" @click="store.clearError()">
            <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
              <path d="M18 6L6 18M6 6l12 12"/>
            </svg>
          </button>
        </div>
      </div>
    </div>


  </div>
</template>

<style scoped>
.msg-list {
  flex: 1;
  display: flex;
  flex-direction: column;
  min-height: 0;
  overflow: hidden;
}
.msg-scroll {
  flex: 1;
  overflow-y: auto;
  overflow-x: hidden;
}
.msg-content {
  max-width: 720px;
  margin: 0 auto;
  padding: 24px 24px 8px;
}

.msg-error {
  display: flex;
  align-items: flex-start;
  justify-content: space-between;
  gap: 12px;
  padding: 12px 16px;
  border-radius: 12px;
  background: #2a1619;
  border: 1px solid #3a1d21;
  color: #ef4444;
  font-size: 13px;
  margin-top: 16px;
  line-height: 1.5;
}
.msg-error-body {
  display: flex;
  flex-direction: column;
  align-items: flex-start;
  gap: 10px;
  flex: 1;
  min-width: 0;
}
.msg-error-dismiss {
  flex-shrink: 0;
  width: 24px;
  height: 24px;
  border-radius: 6px;
  display: flex;
  align-items: center;
  justify-content: center;
  color: #ef4444;
  opacity: 0.6;
  transition: opacity 0.15s, background 0.15s;
}
.msg-error-dismiss:hover {
  opacity: 1;
  background: rgba(220, 38, 38, 0.08);
}
.msg-toolbar {
  display: flex;
  align-items: center;
  justify-content: flex-end;
  padding: 8px 16px;
  flex-shrink: 0;
}
.toolbar-btn {
  display: flex;
  align-items: center;
  gap: 5px;
  font-size: 12px;
  color: #6b7887;
  padding: 4px 6px;
  border-radius: 6px;
  transition: color 0.15s;
}
.toolbar-btn:hover {
  color: #9aa7b4;
}
</style>