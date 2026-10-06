# 流式对话与 WebSocket 协议

## 1. 实现要点（当前）

当前流式由 **QA Agent** 直接调用 OpenAI 兼容 API 的 `stream=True` 完成。

```text
用户 message
  → Retriever + Summarizer（可推送 agent_step）
  → QA Agent LLM stream
       → token_callback → WS type=chunk
       → done_callback  → WS type=done（含 sources）
```

---

## 2. 端点

```text
ws://host/api/qa/ws?session_id={session_id}[&cursor={cursor}]
```

| 参数 | 必填 | 说明 |
|------|------|------|
| `session_id` | 是 | UUID |
| `cursor` | 否 | `message_id+sent_offset` 续写 |

---

## 3. 客户端 → 服务端

| type | 说明 |
|------|------|
| `message` | `{"type":"message","content":"条款编号或中文描述"}` |
| `stop` | 停止当前生成 |
| `ping` | 心跳 |
| `resume` | 带 cursor 续写（可选） |

---

## 4. 服务端 → 客户端

| type | 主要字段 | 说明 |
|------|----------|------|
| `connected` | `session_id`, `streaming_done` | 连接确认 |
| `agent_step` | `step_type`, `content`, `tool_name` | 检索步骤（可折叠 UI） |
| `chunk` | `message_id`, `content`, `done` | 流式 token |
| `done` | `content`, `sources`, `intent`, `has_kg_facts`, `cursor` | 结束 |
| `error` | `message` | 错误 |
| `resume_remaining` | `remaining`, `done` | 断线补发 |

### done.sources 语义（C99 条款）

字段名兼容旧前端，展示时请映射：

| 字段 | 含义 |
|------|------|
| `museum` / `museum_name` | 标准名（ISO/IEC 9899:1999 (C99)） |
| `object_id` | 条款编号 |
| `url` / `detail_url` | 来源链接（N1256 锚点） |
| `accession_number` | chunk_id |

---

## 5. 推荐时序

```text
前端 POST /session → 拿到 session_id
前端 WS 连接
服务端 connected
前端 message
服务端 agent_step（检索中…）
服务端 chunk × N
服务端 done
```

实现：`app/api/ws_router.py`、`app/core/ws_manager.py`、`app/agents/main_agent.py`。
