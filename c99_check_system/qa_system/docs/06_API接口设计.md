# API 接口设计

所有业务接口前缀：`/api/qa`。
Swagger：http://localhost:8000/docs

---

## 1. HTTP REST

### 1.1 创建会话

```http
POST /api/qa/session
```

响应 `201`：

```json
{ "session_id": "550e8400-e29b-41d4-a716-446655440000" }
```

### 1.2 生成（非流式）

```http
POST /api/qa/ask
Content-Type: application/json
```

```json
{
  "question": "6.5.2.2 函数调用的规则",
  "session_id": null
}
```

| 字段 | 必填 | 说明 |
|------|------|------|
| `question` | 是 | 条款编号或中文描述 |
| `session_id` | 否 | 有则多轮 |

响应 `AskResponse` 主要字段：

| 字段 | 说明 |
|------|------|
| `answer` | 生成的 C 测试代码 |
| `intent` | `EXACT_ARTICLE` / `KEYWORD_MATCH` / `LEGAL_RETRIEVE` / `NOT_FOUND` |
| `sources` | 溯源列表 |
| `has_kg_facts` | 是否检索到条款 |
| `has_llm_content` | 是否含模型生成内容 |
| `not_found` | 是否未命中 |

**sources 元素（字段名兼容旧前端，语义为 C99 条款）：**

```json
{
  "museum_name": "ISO/IEC 9899:1999 (C99)",
  "object_id": "6.5.2.2",
  "detail_url": "https://port70.net/~nsz/c/c99/n1256.html#6.5.2.2",
  "accession_number": "c99-6.5.2.2",
  "image_url": null
}
```

PowerShell 示例：

```powershell
Invoke-RestMethod -Uri "http://localhost:8000/api/qa/ask" `
  -Method POST -ContentType "application/json" `
  -Body '{"question":"6.5.2.2 函数调用的规则"}'
```

### 1.3 历史 / 删除 / 反馈

| 方法 | 路径 | 说明 |
|------|------|------|
| GET | `/api/qa/history/{session_id}` | 历史消息 |
| DELETE | `/api/qa/session/{session_id}` | 删除会话 |
| POST | `/api/qa/feedback` | `{ message_id, is_helpful, session_id? }` |

---

## 2. WebSocket

```text
ws://host/api/qa/ws?session_id=xxx
```

消息类型见 [04_流式对话与WebSocket协议](./04_流式对话与WebSocket协议.md)。

---

## 3. 系统级

| 方法 | 路径 | 说明 |
|------|------|------|
| GET | `/` | 服务信息 |
| GET | `/health` | `status` / `faiss_ready` / `llm_model` |

---

## 4. 错误

业务异常多为 HTTP 500 + `detail` 文本；WebSocket 推送 `{"type":"error","message":"..."}`。
