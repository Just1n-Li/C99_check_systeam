<script setup lang="ts">
import { onMounted, ref, watch, computed } from 'vue'
import { marked } from 'marked'
import hljs from 'highlight.js/lib/core'
import ts from 'highlight.js/lib/languages/typescript'
import xml from 'highlight.js/lib/languages/xml'
import py from 'highlight.js/lib/languages/python'
import bash from 'highlight.js/lib/languages/bash'
import json from 'highlight.js/lib/languages/json'
import c from 'highlight.js/lib/languages/c'
import { useChatStore } from '@/stores/chat'
import type { ChatMessage } from '@/types'

hljs.registerLanguage('typescript', ts)
hljs.registerLanguage('javascript', ts)
hljs.registerLanguage('html', xml)
hljs.registerLanguage('python', py)
hljs.registerLanguage('bash', bash)
hljs.registerLanguage('json', json)
hljs.registerLanguage('c', c)

const props = defineProps<{ message: ChatMessage }>()
const store = useChatStore()
const rendered = ref('')

marked.setOptions({ breaks: true, gfm: true })
const renderer = new marked.Renderer()
renderer.code = ({ text, lang }: { text: string; lang?: string }) => {
  const lg = lang && hljs.getLanguage(lang) ? lang : 'plaintext'
  let hl = text
  try { hl = hljs.highlight(text, { language: lg }).value } catch { /* */ }
  return `<pre><code class="hljs language-${lg}">${hl}</code></pre>`
}
marked.use({ renderer })

function doRender() {
  if (!props.message.content || props.message.streaming) {
    rendered.value = ''
    return
  }
  rendered.value = marked.parse(props.message.content) as string
}

onMounted(doRender)
watch(() => [props.message.content, props.message.streaming], doRender)

const contentNote = computed(() => {
  if (props.message.streaming) return ''
  if (props.message.has_kg_facts && props.message.has_llm_content) {
    return '以上测试代码由大语言模型基于 C99 条款检索结果生成，编译与运行行为请以实际编译器为准。'
  }
  if (props.message.has_llm_content) {
    return '以上内容由大语言模型生成。'
  }
  return ''
})

const copied = ref(false)
let copiedTimer: ReturnType<typeof setTimeout> | null = null

const isAssistantReply = computed(() =>
  props.message.role === 'assistant' && props.message.message_type !== 'rag_agent',
)

const canCopy = computed(() => {
  if (props.message.message_type === 'rag_agent') return false
  if (props.message.streaming) return false
  return Boolean(props.message.content?.trim())
})

const dbMessageId = computed(() => {
  const match = props.message.id.match(/^(?:done|stream)-(\d+)$/)
  if (!match) return null
  const n = Number(match[1])
  return Number.isNaN(n) ? null : n
})

const canFeedback = computed(() =>
  isAssistantReply.value
  && !props.message.streaming
  && Boolean(props.message.content?.trim())
  && dbMessageId.value != null,
)

const uniqueSources = computed(() => {
  if (!props.message.sources?.length) return []
  const seen = new Set<string>()
  return props.message.sources.filter((s) => {
    const url = (s.url || (s as { detail_url?: string }).detail_url || '').trim()
    if (!url) return false
    const key = url.toLowerCase()
    if (seen.has(key)) return false
    seen.add(key)
    return true
  }).map((s) => ({
    ...s,
    url: (s.url || (s as { detail_url?: string }).detail_url || '').trim(),
    museum: s.museum || (s as { museum_name?: string }).museum_name || '来源',
  }))
})

const SOURCES_PREVIEW_COUNT = 3
const sourcesExpanded = ref(false)

watch(() => props.message.id, () => {
  sourcesExpanded.value = false
})

const displaySources = computed(() => {
  if (sourcesExpanded.value || uniqueSources.value.length <= SOURCES_PREVIEW_COUNT) {
    return uniqueSources.value
  }
  return uniqueSources.value.slice(0, SOURCES_PREVIEW_COUNT)
})

const hiddenSourcesCount = computed(() =>
  Math.max(0, uniqueSources.value.length - SOURCES_PREVIEW_COUNT),
)

function retryQuestion() {
  void store.retryQuestion(props.message.content, props.message.id)
}

function retryAnswerForUserQuestion() {
  const msgs = store.messages
  const idx = msgs.findIndex((m) => m.id === props.message.id)
  if (idx < 0) return
  for (let i = idx - 1; i >= 0; i--) {
    const m = msgs[i]!
    if (m.role === 'user') {
      void store.retryQuestion(m.content, m.id)
      return
    }
  }
}

function copyPayload(): string {
  const parts = [props.message.content.trim()]
  if (contentNote.value) parts.push('', contentNote.value)
  return parts.join('\n')
}

async function copyMessage() {
  if (!canCopy.value) return
  const text = copyPayload()
  try {
    await navigator.clipboard.writeText(text)
  } catch {
    const ta = document.createElement('textarea')
    ta.value = text
    ta.style.position = 'fixed'
    ta.style.left = '-9999px'
    document.body.appendChild(ta)
    ta.select()
    document.execCommand('copy')
    document.body.removeChild(ta)
  }
  copied.value = true
  if (copiedTimer) clearTimeout(copiedTimer)
  copiedTimer = setTimeout(() => { copied.value = false }, 2000)
}

function submitFeedback(isHelpful: boolean) {
  if (!canFeedback.value || props.message.feedback || props.message.feedbackSubmitting) return
  void store.submitFeedback(props.message.id, isHelpful)
}

const ICONS = {
  reasoning: `<svg viewBox="0 0 24 24" width="14" height="14" fill="none" stroke="currentColor" stroke-width="1.75" stroke-linecap="round" stroke-linejoin="round"><path d="M9 18h6"/><path d="M10 22h4"/><path d="M12 2a7 7 0 0 0-4 12.7c.6.5 1 1.2 1 2v.3h6v-.3c0-.8.4-1.5 1-2A7 7 0 0 0 12 2z"/></svg>`,
  mysql: `<svg viewBox="0 0 24 24" width="14" height="14" fill="none" stroke="currentColor" stroke-width="1.75" stroke-linecap="round" stroke-linejoin="round"><ellipse cx="12" cy="5" rx="9" ry="3"/><path d="M3 5v6c0 1.7 4 3 9 3s9-1.3 9-3V5"/><path d="M3 11v6c0 1.7 4 3 9 3s9-1.3 9-3v-6"/></svg>`,
  neo4j_query: `<svg viewBox="0 0 24 24" width="14" height="14" fill="none" stroke="currentColor" stroke-width="1.75" stroke-linecap="round" stroke-linejoin="round"><circle cx="5" cy="6" r="2"/><circle cx="19" cy="6" r="2"/><circle cx="12" cy="18" r="2"/><circle cx="12" cy="12" r="1.5" fill="currentColor" stroke="none"/><line x1="6.5" y1="7" x2="11" y2="11"/><line x1="17.5" y1="7" x2="13" y2="11"/><line x1="12" y1="13.5" x2="12" y2="16"/></svg>`,
  neo4j_schema: `<svg viewBox="0 0 24 24" width="14" height="14" fill="none" stroke="currentColor" stroke-width="1.75" stroke-linecap="round" stroke-linejoin="round"><rect x="4" y="4" width="16" height="16" rx="2"/><line x1="8" y1="9" x2="16" y2="9"/><line x1="8" y1="13" x2="16" y2="13"/><line x1="8" y1="17" x2="13" y2="17"/></svg>`,
  neo4j_sample: `<svg viewBox="0 0 24 24" width="14" height="14" fill="none" stroke="currentColor" stroke-width="1.75" stroke-linecap="round" stroke-linejoin="round"><circle cx="11" cy="11" r="7"/><line x1="21" y1="21" x2="16.5" y2="16.5"/></svg>`,
  neo4j_count: `<svg viewBox="0 0 24 24" width="14" height="14" fill="none" stroke="currentColor" stroke-width="1.75" stroke-linecap="round" stroke-linejoin="round"><line x1="9" y1="3" x2="9" y2="21"/><line x1="15" y1="3" x2="15" y2="21"/><line x1="3" y1="9" x2="21" y2="9"/><line x1="3" y1="15" x2="21" y2="15"/></svg>`,
  document: `<svg viewBox="0 0 24 24" width="14" height="14" fill="none" stroke="currentColor" stroke-width="1.75" stroke-linecap="round" stroke-linejoin="round"><path d="M14 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V8z"/><polyline points="14 2 14 8 20 8"/><line x1="8" y1="13" x2="16" y2="13"/><line x1="8" y1="17" x2="14" y2="17"/></svg>`,
  tool_call_fallback: `<svg viewBox="0 0 24 24" width="14" height="14" fill="none" stroke="currentColor" stroke-width="1.75" stroke-linecap="round" stroke-linejoin="round"><polyline points="16 18 22 12 16 6"/><polyline points="8 6 2 12 8 18"/></svg>`,
  tool_result_fallback: `<svg viewBox="0 0 24 24" width="14" height="14" fill="none" stroke="currentColor" stroke-width="1.75" stroke-linecap="round" stroke-linejoin="round"><path d="M22 11.08V12a10 10 0 1 1-5.93-9.14"/><polyline points="22 4 12 14.01 9 11.01"/></svg>`,
}

const NEO4J_TOOLS = new Set([
  'query_neo4j', 'get_graph_schema', 'explore_graph_sample', 'count_nodes_by_label',
])

function getStepIcon(step_type: string, tool_name?: string): string {
  if ((step_type === 'tool_call' || step_type === 'tool_result') && tool_name) {
    switch (tool_name) {
      case 'execute_sql': return ICONS.mysql
      case 'query_neo4j': return ICONS.neo4j_query
      case 'get_graph_schema': return ICONS.neo4j_schema
      case 'explore_graph_sample': return ICONS.neo4j_sample
      case 'count_nodes_by_label': return ICONS.neo4j_count
      case 'summarize_result': return ICONS.document
    }
  }
  switch (step_type) {
    case 'reasoning': return ICONS.reasoning
    case 'tool_call': return ICONS.tool_call_fallback
    case 'tool_result': return ICONS.tool_result_fallback
    default: return ICONS.tool_result_fallback
  }
}

function getStepClass(step_type: string, tool_name?: string): string {
  const classes: string[] = [`rag-step--${step_type}`]
  if (tool_name) {
    if (tool_name === 'execute_sql') classes.push('rag-step--mysql')
    else if (NEO4J_TOOLS.has(tool_name)) classes.push('rag-step--neo4j')
    else if (tool_name === 'summarize_result') classes.push('rag-step--summary')
  }
  return classes.join(' ')
}

const ragExpanded = ref(false)

function isLatestRagMessage(): boolean {
  const msgs = store.messages
  for (let i = msgs.length - 1; i >= 0; i--) {
    if (msgs[i]?.message_type === 'rag_agent') {
      return msgs[i]!.id === props.message.id
    }
  }
  return false
}

const ragInProgress = computed(() => store.isStreaming && isLatestRagMessage())

const ragStepCount = computed(() => props.message.agent_steps?.length ?? 0)

const ragLastStepPreview = computed(() => {
  const steps = props.message.agent_steps
  if (!steps?.length) return ''
  const text = steps[steps.length - 1]!.content.trim()
  return text.length > 48 ? `${text.slice(0, 48)}…` : text
})

function toggleRagExpanded() {
  ragExpanded.value = !ragExpanded.value
}

watch(() => props.message.id, () => {
  ragExpanded.value = false
})

watch(
  ragInProgress,
  (active) => {
    if (active) ragExpanded.value = true
  },
  { immediate: true },
)

watch(
  () => store.isStreaming,
  (streaming, wasStreaming) => {
    if (wasStreaming && !streaming && isLatestRagMessage()) {
      ragExpanded.value = false
    }
  },
)
</script>

<template>
  <div class="msg-item fade-in">
    <!-- 用户消息 -->
    <div v-if="message.role === 'user'" class="msg-row msg-row--user">
      <div class="msg-user-wrap">
        <div class="msg-user-actions">
          <button
            v-if="canCopy"
            type="button"
            class="msg-icon-btn"
            :title="copied ? '已复制' : '复制'"
            aria-label="复制问题"
            @click="copyMessage"
          >
            <svg v-if="!copied" width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
              <rect x="9" y="9" width="13" height="13" rx="2"/><path d="M5 15H4a2 2 0 0 1-2-2V4a2 2 0 0 1 2-2h9a2 2 0 0 1 2 2v1"/>
            </svg>
            <svg v-else width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5">
              <polyline points="20 6 9 17 4 12"/>
            </svg>
          </button>
          <button
            type="button"
            class="msg-icon-btn"
            :disabled="store.isStreaming"
            title="重新提问"
            aria-label="重新提问"
            @click="retryQuestion"
          >
            <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
              <polyline points="23 4 23 10 17 10"/>
              <path d="M20.49 15a9 9 0 1 1-2.12-9.36L23 10"/>
            </svg>
          </button>
        </div>
        <div class="msg-bubble-user">{{ message.content }}</div>
      </div>
    </div>

    <!-- RAG Agent -->
    <div v-else-if="message.message_type === 'rag_agent'" class="msg-row msg-row--assistant">
      <div class="msg-avatar msg-avatar--rag" aria-hidden="true">
        <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
          <path d="M12 2L2 7l10 5 10-5-10-5z"/><path d="M2 17l10 5 10-5"/><path d="M2 12l10 5 10-5"/>
        </svg>
      </div>
      <div class="msg-rag-card">
        <div class="rag-header" :class="{ 'rag-header--expanded': ragExpanded }">
          <button
            type="button"
            class="rag-header-toggle"
            :aria-expanded="ragExpanded"
            aria-controls="rag-steps-panel"
            @click="toggleRagExpanded"
          >
            <svg
              class="rag-chevron"
              :class="{ 'rag-chevron--open': ragExpanded }"
              width="14"
              height="14"
              viewBox="0 0 24 24"
              fill="none"
              stroke="currentColor"
              stroke-width="2"
              stroke-linecap="round"
              stroke-linejoin="round"
              aria-hidden="true"
            >
              <polyline points="9 18 15 12 9 6"/>
            </svg>
            <span class="rag-header-label">RAG 检索过程</span>
            <span v-if="ragStepCount" class="rag-header-count">{{ ragStepCount }} 步</span>
            <span v-if="ragInProgress" class="rag-header-badge">检索中</span>
            <span v-else class="rag-header-badge rag-header-badge--done">已完成</span>
            <span v-if="!ragExpanded && ragLastStepPreview" class="rag-header-preview">
              {{ ragLastStepPreview }}
            </span>
          </button>
          <div class="rag-header-actions">
            <button
              type="button"
              class="msg-icon-btn"
              :disabled="store.isStreaming"
              title="重新提问"
              aria-label="重新提问"
              @click.stop="retryAnswerForUserQuestion"
            >
              <svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                <polyline points="23 4 23 10 17 10"/>
                <path d="M20.49 15a9 9 0 1 1-2.12-9.36L23 10"/>
              </svg>
            </button>
          </div>
        </div>
        <Transition name="rag-collapse">
          <div
            v-show="ragExpanded"
            id="rag-steps-panel"
            class="rag-steps"
          >
            <div
              v-for="step in message.agent_steps"
              :key="step.id"
              class="rag-step"
              :class="getStepClass(step.step_type, step.tool_name)"
            >
              <span class="rag-step-icon" v-html="getStepIcon(step.step_type, step.tool_name)" />
              <span class="rag-step-content">{{ step.content }}</span>
            </div>
          </div>
        </Transition>
      </div>
    </div>

    <!-- 助手回答 -->
    <div v-else class="msg-row msg-row--assistant">
      <div class="msg-avatar" aria-hidden="true">
        <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.75">
          <path d="M3 9h18v10a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2V9z"/>
          <path d="M3 9l2.5-5h13L21 9"/>
          <path d="M12 14v3"/>
        </svg>
      </div>

      <div class="msg-assistant-wrap">
        <div class="msg-assistant-card">
          <div
            v-if="message.content && !message.streaming"
            class="md-body"
            v-html="rendered"
          />
          <div
            v-else-if="message.content && message.streaming"
            class="msg-stream-text"
          >{{ message.content }}<span class="stream-cursor" /></div>
          <div v-else-if="message.streaming" class="msg-loading">
            <span class="msg-loading-dot" /><span class="msg-loading-dot" /><span class="msg-loading-dot" />
            正在生成回复…
          </div>

          <p v-if="contentNote" class="msg-content-note">{{ contentNote }}</p>
        </div>

        <div
          v-if="(uniqueSources.length && !message.streaming) || canFeedback || canCopy || isAssistantReply"
          class="msg-assistant-footer"
        >
          <div v-if="uniqueSources.length && !message.streaming" class="msg-sources">
            <div class="msg-sources-head">
              <span class="msg-sources-label">来源</span>
              <span v-if="uniqueSources.length > 1" class="msg-sources-count">{{ uniqueSources.length }} 条</span>
            </div>
            <div class="msg-sources-links">
              <a
                v-for="(src, i) in displaySources"
                :key="src.url + i"
                :href="src.url"
                target="_blank"
                rel="noopener noreferrer"
                class="msg-source-link"
              >
                <svg width="10" height="10" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5">
                  <path d="M18 13v6a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2V8a2 2 0 0 1 2-2h6"/>
                  <polyline points="15 3 21 3 21 9"/><line x1="10" y1="14" x2="21" y2="3"/>
                </svg>
                {{ src.title ? `${src.museum} · ${src.title}` : src.museum }}
              </a>
            </div>
            <button
              v-if="hiddenSourcesCount && !sourcesExpanded"
              type="button"
              class="msg-sources-toggle"
              @click="sourcesExpanded = true"
            >
              展开其余 {{ hiddenSourcesCount }} 个来源
            </button>
            <button
              v-else-if="sourcesExpanded && uniqueSources.length > SOURCES_PREVIEW_COUNT"
              type="button"
              class="msg-sources-toggle"
              @click="sourcesExpanded = false"
            >
              收起来源
            </button>
          </div>

          <div
            v-if="canFeedback || canCopy || isAssistantReply"
            class="msg-assistant-toolbar"
          >
            <div v-if="canFeedback" class="msg-feedback-bar">
              <span class="msg-feedback-label">这条回答有帮助吗？</span>
              <div class="msg-feedback-actions">
                <button
                  type="button"
                  class="msg-feedback-btn"
                  :class="{
                    'msg-feedback-btn--active': message.feedback === 'helpful',
                    'msg-feedback-btn--disabled': Boolean(message.feedback) && message.feedback !== 'helpful',
                  }"
                  :disabled="Boolean(message.feedback) || message.feedbackSubmitting"
                  aria-label="标记为有帮助"
                  @click="submitFeedback(true)"
                >
                  <svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                    <path d="M14 9V5a3 3 0 0 0-3-3l-4 9v11h11.28a2 2 0 0 0 2-1.7l1.38-9a2 2 0 0 0-2-2.3H14z"/>
                    <path d="M7 22H4a2 2 0 0 1-2-2v-7a2 2 0 0 1 2-2h3"/>
                  </svg>
                  有帮助
                </button>
                <button
                  type="button"
                  class="msg-feedback-btn msg-feedback-btn--negative"
                  :class="{
                    'msg-feedback-btn--active': message.feedback === 'inaccurate',
                    'msg-feedback-btn--disabled': Boolean(message.feedback) && message.feedback !== 'inaccurate',
                  }"
                  :disabled="Boolean(message.feedback) || message.feedbackSubmitting"
                  aria-label="标记为不准确"
                  @click="submitFeedback(false)"
                >
                  <svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                    <path d="M10 15v4a3 3 0 0 0 3 3l4-9V2H5.72a2 2 0 0 0-2 1.7l-1.38 9a2 2 0 0 0 2 2.3H10z"/>
                    <path d="M17 2h2.67A2.31 2.31 0 0 1 22 4v7a2.31 2.31 0 0 1-2.33 2H17"/>
                  </svg>
                  不准确
                </button>
              </div>
              <span v-if="message.feedbackSubmitting" class="msg-feedback-hint">提交中…</span>
              <span v-else-if="message.feedback === 'helpful'" class="msg-feedback-hint msg-feedback-hint--ok">感谢反馈</span>
              <span v-else-if="message.feedback === 'inaccurate'" class="msg-feedback-hint">已记录，将用于改进</span>
            </div>

            <div v-if="canCopy || isAssistantReply" class="msg-actions">
              <button
                v-if="isAssistantReply"
                type="button"
                class="msg-icon-btn"
                :disabled="store.isStreaming"
                title="重新提问"
                aria-label="重新提问"
                @click="retryAnswerForUserQuestion"
              >
                <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                  <polyline points="23 4 23 10 17 10"/>
                  <path d="M20.49 15a9 9 0 1 1-2.12-9.36L23 10"/>
                </svg>
              </button>
              <button
                v-if="canCopy"
                type="button"
                class="msg-icon-btn"
                :title="copied ? '已复制' : '复制'"
                aria-label="复制回答"
                @click="copyMessage"
              >
                <svg v-if="!copied" width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                  <rect x="9" y="9" width="13" height="13" rx="2"/><path d="M5 15H4a2 2 0 0 1-2-2V4a2 2 0 0 1 2-2h9a2 2 0 0 1 2 2v1"/>
                </svg>
                <svg v-else width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5">
                  <polyline points="20 6 9 17 4 12"/>
                </svg>
              </button>
            </div>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<style scoped>
.msg-item {
  padding: 10px 0;
}

.msg-row {
  display: flex;
  gap: 12px;
  align-items: flex-start;
}
.msg-row--user {
  justify-content: flex-end;
}

.msg-avatar {
  flex-shrink: 0;
  width: 32px;
  height: 32px;
  border-radius: 10px;
  display: flex;
  align-items: center;
  justify-content: center;
  background: linear-gradient(135deg, rgba(34,211,238,0.12) 0%, rgba(34,211,238,0.2) 100%);
  color: var(--color-accent);
  border: 1px solid rgba(34,211,238,0.35);
  margin-top: 2px;
}
.msg-avatar--rag {
  background: linear-gradient(135deg, rgba(167,139,250,0.08) 0%, rgba(167,139,250,0.15) 100%);
  color: #a78bfa;
  border-color: rgba(167,139,250,0.3);
}

.msg-user-wrap {
  display: flex;
  align-items: flex-end;
  gap: 6px;
  max-width: min(75%, 560px);
}
.msg-user-actions {
  display: flex;
  flex-direction: column;
  gap: 2px;
  flex-shrink: 0;
  opacity: 0;
  transition: opacity 0.15s;
}
.msg-user-wrap:hover .msg-user-actions,
.msg-user-actions:focus-within {
  opacity: 1;
}
.msg-icon-btn {
  flex-shrink: 0;
  display: flex;
  align-items: center;
  justify-content: center;
  width: 28px;
  height: 28px;
  border-radius: 8px;
  color: var(--color-text-subtle);
  background: transparent;
  border: 1px solid transparent;
  transition: color 0.15s, background 0.15s, border-color 0.15s;
}
.msg-icon-btn:hover:not(:disabled) {
  color: #9aa7b4;
  background: #1a212b;
  border-color: #232b36;
}
.msg-icon-btn:disabled {
  opacity: 0.45;
  cursor: not-allowed;
}
.msg-bubble-user {
  background: linear-gradient(135deg, #0e7490 0%, #155e75 100%);
  color: #ecfeff;
  border-radius: 18px 18px 4px 18px;
  padding: 12px 16px;
  font-size: 14px;
  line-height: 1.65;
  white-space: pre-wrap;
  word-break: break-word;
  box-shadow: 0 2px 12px rgba(0, 0, 0, 0.08);
}

.msg-assistant-wrap {
  flex: 1;
  min-width: 0;
  max-width: min(85%, 640px);
}
.msg-assistant-card {
  background: #161b22;
  border: 1px solid var(--color-border);
  border-radius: 4px 16px 16px 16px;
  padding: 14px 18px;
  box-shadow: 0 1px 8px rgba(0, 0, 0, 0.04);
}

.msg-assistant-footer {
  margin-top: 8px;
  padding-left: 2px;
  display: flex;
  flex-direction: column;
  gap: 10px;
}

.msg-assistant-toolbar {
  display: flex;
  flex-wrap: wrap;
  align-items: center;
  justify-content: space-between;
  gap: 8px 12px;
}

.msg-stream-text {
  font-size: 15px;
  line-height: 1.75;
  color: #c9d4de;
  white-space: pre-wrap;
  word-break: break-word;
}

.msg-content-note {
  margin: 10px 0 0;
  padding-top: 10px;
  border-top: 1px dashed #232b36;
  font-size: 11.5px;
  line-height: 1.5;
  color: var(--color-text-subtle);
}

.msg-feedback-bar {
  display: flex;
  flex-wrap: wrap;
  align-items: center;
  gap: 8px 12px;
  flex: 1;
  min-width: 0;
}
.msg-feedback-label {
  font-size: 12.5px;
  color: var(--color-text-muted);
  font-weight: 500;
}
.msg-feedback-actions {
  display: flex;
  flex-wrap: wrap;
  gap: 6px;
}

.msg-loading {
  display: flex;
  align-items: center;
  gap: 6px;
  font-size: 13px;
  color: var(--color-text-subtle);
  padding: 2px 0;
}
.msg-loading-dot {
  width: 5px;
  height: 5px;
  border-radius: 50%;
  background: var(--color-accent);
  animation: loading-bounce 1.2s ease-in-out infinite;
}
.msg-loading-dot:nth-child(2) { animation-delay: 0.15s; }
.msg-loading-dot:nth-child(3) { animation-delay: 0.3s; }
@keyframes loading-bounce {
  0%, 80%, 100% { opacity: 0.3; transform: scale(0.8); }
  40% { opacity: 1; transform: scale(1); }
}

.msg-rag-card {
  flex: 1;
  min-width: 0;
  max-width: 90%;
  background: #161b22;
  border: 1px solid #232b36;
  border-radius: 12px;
  overflow: hidden;
  box-shadow: 0 1px 8px rgba(0, 0, 0, 0.04);
}
.rag-header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  gap: 8px;
  padding: 6px 8px 6px 6px;
  background: linear-gradient(to right, #141a21, rgba(167,139,250,0.08));
}
.rag-header--expanded {
  border-bottom: 1px solid #232b36;
}
.rag-header-toggle {
  display: flex;
  align-items: center;
  flex-wrap: wrap;
  gap: 6px 8px;
  flex: 1;
  min-width: 0;
  padding: 4px 8px;
  border-radius: 8px;
  text-align: left;
  color: inherit;
  transition: background 0.15s;
}
.rag-header-toggle:hover {
  background: rgba(124, 58, 237, 0.06);
}
.rag-chevron {
  flex-shrink: 0;
  color: #a78bfa;
  transition: transform 0.2s cubic-bezier(0.16, 1, 0.3, 1);
}
.rag-chevron--open {
  transform: rotate(90deg);
}
.rag-header-label {
  font-size: 13px;
  font-weight: 600;
  color: #e6edf3;
  white-space: nowrap;
}
.rag-header-count {
  font-size: 11px;
  color: #9aa7b4;
  background: #1a212b;
  padding: 1px 7px;
  border-radius: 999px;
  white-space: nowrap;
}
.rag-header-preview {
  flex: 1 1 100%;
  font-size: 12px;
  color: #9aa7b4;
  line-height: 1.4;
  padding-left: 22px;
  overflow: hidden;
  text-overflow: ellipsis;
  white-space: nowrap;
}
.rag-header-badge {
  font-size: 11px;
  font-weight: 500;
  color: #a78bfa;
  background: rgba(167,139,250,0.15);
  padding: 2px 8px;
  border-radius: 999px;
  white-space: nowrap;
}
.rag-header-badge--done {
  color: #059669;
  background: rgba(34,211,238,0.12);
}
.rag-header-actions {
  display: flex;
  align-items: center;
  gap: 8px;
  flex-shrink: 0;
}
.rag-collapse-enter-active,
.rag-collapse-leave-active {
  transition: opacity 0.2s ease, max-height 0.28s cubic-bezier(0.16, 1, 0.3, 1);
  overflow: hidden;
}
.rag-collapse-enter-from,
.rag-collapse-leave-to {
  opacity: 0;
  max-height: 0;
}
.rag-collapse-enter-to,
.rag-collapse-leave-from {
  opacity: 1;
  max-height: 640px;
}
.rag-steps { padding: 4px 0; }
.rag-step {
  display: flex;
  align-items: flex-start;
  gap: 10px;
  padding: 8px 14px;
  font-size: 12.5px;
  line-height: 1.55;
  border-bottom: 1px solid #1a212b;
}
.rag-step:last-child { border-bottom: none; }
.rag-step-icon {
  flex-shrink: 0;
  width: 18px;
  height: 18px;
  display: flex;
  align-items: center;
  justify-content: center;
  margin-top: 1px;
  color: #9aa7b4;
}
.rag-step-content {
  flex: 1;
  color: #e6edf3;
  word-break: break-word;
}
.rag-step--reasoning { background: linear-gradient(to right, rgba(251,191,36,0.06) 0%, transparent 60%); }
.rag-step--reasoning .rag-step-icon { color: #fbbf24; }
.rag-step--mysql { background: linear-gradient(to right, rgba(59,130,246,0.08) 0%, transparent 60%); }
.rag-step--mysql .rag-step-icon { color: #60a5fa; }
.rag-step--neo4j { background: linear-gradient(to right, rgba(34,211,238,0.12) 0%, transparent 60%); }
.rag-step--neo4j .rag-step-icon { color: #059669; }
.rag-step--summary { background: linear-gradient(to right, #1a212b 0%, transparent 60%); }
.rag-step--summary .rag-step-icon { color: #9aa7b4; }

.msg-sources {
  display: flex;
  flex-direction: column;
  gap: 8px;
}
.msg-sources-head {
  display: flex;
  align-items: center;
  gap: 8px;
}
.msg-sources-label {
  font-size: 11.5px;
  font-weight: 600;
  color: var(--color-text-subtle);
  letter-spacing: 0.02em;
}
.msg-sources-count {
  font-size: 11px;
  color: var(--color-text-muted);
  background: #1a212b;
  border-radius: 999px;
  padding: 1px 8px;
}
.msg-sources-toggle {
  align-self: flex-start;
  font-size: 12px;
  color: var(--color-text-muted);
  padding: 2px 0;
  transition: color 0.15s;
}
.msg-sources-toggle:hover {
  color: var(--color-accent);
}
.msg-sources-links {
  display: flex;
  flex-wrap: wrap;
  gap: 8px;
}
.msg-source-link {
  display: inline-flex;
  align-items: center;
  gap: 5px;
  font-size: 12px;
  color: var(--color-text-muted);
  background: #141a21;
  border: 1px solid var(--color-border);
  border-radius: 999px;
  padding: 4px 10px;
  text-decoration: none;
  transition: color 0.15s, border-color 0.15s, background 0.15s;
}
.msg-source-link:hover {
  color: var(--color-accent);
  border-color: rgba(34,211,238,0.35);
  background: rgba(34,211,238,0.12);
}

.msg-actions {
  display: flex;
  align-items: center;
  gap: 4px;
  flex-shrink: 0;
}

.msg-action-btn,
.msg-feedback-btn {
  display: inline-flex;
  align-items: center;
  gap: 4px;
  font-size: 12px;
  color: var(--color-text-subtle);
  padding: 5px 8px;
  border-radius: 8px;
  border: 1px solid transparent;
  transition: color 0.15s, background 0.15s, border-color 0.15s;
}
.msg-action-btn:focus-visible,
.msg-icon-btn:focus-visible {
  opacity: 1;
}
.msg-action-btn:hover,
.msg-feedback-btn:hover:not(:disabled) {
  color: #9aa7b4;
  background: #1a212b;
  border-color: #232b36;
}

.msg-feedback-btn--active {
  color: var(--color-accent);
  background: rgba(34,211,238,0.12);
  border-color: rgba(34,211,238,0.35);
}
.msg-feedback-btn--negative.msg-feedback-btn--active {
  color: #ef4444;
  background: #2a1619;
  border-color: #3a1d21;
}
.msg-feedback-btn--disabled {
  opacity: 0.35;
}
.msg-feedback-btn:disabled {
  cursor: default;
}

.msg-feedback-hint {
  font-size: 11.5px;
  color: var(--color-text-subtle);
}
.msg-feedback-hint--ok {
  color: var(--color-accent);
}
</style>
