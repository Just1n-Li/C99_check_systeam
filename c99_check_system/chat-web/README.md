# chat-web（前端）

Vue 3 + Vite 前端界面。后端 RAG + Agent **已就绪**，本目录负责页面展示与联调。

## 对接后端

默认后端：`http://localhost:8000`

| 用途 | 地址 |
|------|------|
| HTTP | `http://localhost:8000/api/qa` |
| WebSocket | `ws://localhost:8000/api/qa/ws?session_id=...` |
| API 文档 | http://localhost:8000/docs |

请在 `src/api/chat.ts` 中确认 `HTTP_BASE` / `WS_BASE`。

联调前请先启动后端：

```powershell
cd ../qa_system
.\.venv\Scripts\Activate.ps1
uvicorn main:app --reload --host 0.0.0.0 --port 8000
```

完整接口与溯源字段说明见仓库根目录 [README.md](../README.md)。

### 展示说明（领域：C99 编译器测试代码生成）

- 输入 C99 条款编号（如 `6.5.2.2`）或中文描述，流式展示生成的 C 测试代码
- 来源字段沿用旧协议名：`sources[].museum_name` → 标准 / 条款名，`sources[].detail_url` → N1256 原文锚点
- 生成代码使用 C 语法高亮（hljs 已注册 `c`）

## 本地启动

```sh
npm install
npm run dev
```

默认 http://localhost:5173

```sh
npm run build
npm run preview
```

## 推荐 IDE

[VS Code](https://code.visualstudio.com/) + [Vue (Official)](https://marketplace.visualstudio.com/items?itemName=Vue.volar)
