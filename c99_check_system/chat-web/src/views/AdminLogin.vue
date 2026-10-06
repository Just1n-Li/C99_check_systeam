<script setup lang="ts">
import { ref } from 'vue'
import { useRouter } from 'vue-router'
import { adminLogin } from '@/api/admin'

const router = useRouter()
const password = ref('')
const error = ref('')
const loading = ref(false)

async function handleLogin() {
  if (!password.value.trim() || loading.value) return
  error.value = ''
  loading.value = true
  try {
    await adminLogin(password.value)
    await router.replace('/admin')
  } catch (e) {
    error.value = e instanceof Error ? e.message : '登录失败'
  } finally {
    loading.value = false
  }
}
</script>

<template>
  <div class="admin-login">
    <div class="login-card">
      <div class="login-icon" aria-hidden="true">
        <svg width="28" height="28" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.75">
          <rect x="3" y="11" width="18" height="11" rx="2"/>
          <path d="M7 11V7a5 5 0 0 1 10 0v4"/>
        </svg>
      </div>
      <h1 class="login-title">管理员入口</h1>
      <p class="login-sub">反馈统计与不准确回答审核（课程演示用，无需普通用户注册登录）</p>

      <form class="login-form" @submit.prevent="handleLogin">
        <label class="login-label" for="admin-pwd">管理员密码</label>
        <input
          id="admin-pwd"
          v-model="password"
          type="password"
          class="login-input"
          placeholder="请输入管理员密码"
          autocomplete="current-password"
        />
        <p v-if="error" class="login-error">{{ error }}</p>
        <button type="submit" class="login-btn" :disabled="loading || !password.trim()">
          {{ loading ? '登录中…' : '登录' }}
        </button>
      </form>

      <router-link to="/" class="login-back">← 返回问答首页</router-link>
    </div>
  </div>
</template>

<style scoped>
.admin-login {
  min-height: 100%;
  display: flex;
  align-items: center;
  justify-content: center;
  padding: 24px;
  background:
    radial-gradient(ellipse 70% 50% at 50% -10%, rgba(34, 211, 238, 0.08), transparent),
    var(--color-bg);
}
.login-card {
  width: 100%;
  max-width: 400px;
  background: #161b22;
  border: 1px solid var(--color-border);
  border-radius: 16px;
  padding: 32px 28px;
  box-shadow: var(--shadow-input);
}
.login-icon {
  width: 52px;
  height: 52px;
  border-radius: 14px;
  display: flex;
  align-items: center;
  justify-content: center;
  background: rgba(34,211,238,0.12);
  color: var(--color-accent);
  margin-bottom: 16px;
}
.login-title {
  font-size: 20px;
  font-weight: 700;
  color: var(--color-text);
  margin-bottom: 6px;
}
.login-sub {
  font-size: 13px;
  color: var(--color-text-muted);
  line-height: 1.55;
  margin-bottom: 24px;
}
.login-form {
  display: flex;
  flex-direction: column;
  gap: 8px;
}
.login-label {
  font-size: 13px;
  font-weight: 500;
  color: var(--color-text-muted);
}
.login-input {
  border: 1px solid var(--color-border);
  border-radius: 10px;
  padding: 10px 12px;
  font-size: 14px;
  outline: none;
  transition: border-color 0.15s;
}
.login-input:focus {
  border-color: rgba(34,211,238,0.35);
}
.login-error {
  font-size: 13px;
  color: #ef4444;
  margin-top: 4px;
}
.login-btn {
  margin-top: 8px;
  padding: 11px;
  border-radius: 10px;
  background: var(--color-accent);
  color: #fff;
  font-size: 14px;
  font-weight: 600;
  transition: background 0.15s;
}
.login-btn:hover:not(:disabled) {
  background: var(--color-accent-hover);
}
.login-btn:disabled {
  opacity: 0.55;
  cursor: not-allowed;
}
.login-back {
  display: inline-block;
  margin-top: 20px;
  font-size: 13px;
  color: var(--color-text-subtle);
  text-decoration: none;
}
.login-back:hover {
  color: var(--color-accent);
}
</style>
