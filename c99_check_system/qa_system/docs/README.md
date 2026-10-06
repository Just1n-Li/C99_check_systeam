# 设计文档索引

本目录为 **C99 编译器测试代码生成**系统后端（RAG + Agent）的设计说明。
根目录 [README.md](../../README.md) 含前端对接速查；建议先读本文档 **00**。

---

## 文档列表

| 序号 | 文件 | 内容 |
|------|------|------|
| 00 | [子系统整体介绍](./00_子系统整体介绍.md) | 定位、目标、组成、流程（**建议先读**） |
| 01 | [系统架构](./01_系统架构.md) | 架构图、技术栈、配置、数据流 |
| 02 | [数据与存储设计](./02_数据库设计.md) | 数据 / FAISS / 可选 ai_history |
| 03 | [会话与历史管理](./03_会话与历史管理.md) | session、history、cursor 续写 |
| 04 | [流式对话与 WebSocket](./04_流式对话与WebSocket协议.md) | WS 消息协议、时序 |
| 05 | [Agent 与 Tool 设计](./05_Agent与Tool设计.md) | 检索 / 总结 / 生成三 Agent、FAISS Tool |
| 06 | [API 接口设计](./06_API接口设计.md) | REST + WebSocket、示例 |
| 07 | [外部对接指南](./07_外部子系统对接指南.md) | 前端 / 其他系统调用说明 |
| 09 | [联调说明](./09_联调说明.md) | 本机 / 局域网联调步骤 |
| 10 | [功能测试用例](./10_功能测试用例.md) | 后端完善度验收用例 |

---

## 快速导航

### 前端

- 接口与溯源字段 → [06](./06_API接口设计.md)、根 README
- WebSocket → [04](./04_流式对话与WebSocket协议.md)
- 联调 → [09](./09_联调说明.md)

### 后端

- Agent / FAISS → [05](./05_Agent与Tool设计.md)、[01](./01_系统架构.md)
- 数据接入 → `data_c99/`、[02](./02_数据库设计.md)
- 自测用例 → [10](./10_功能测试用例.md)

---

## 关键路径（当前）

```text
qa_system/
├── main.py / config.py / .env
├── data_c99/             # C99 条款数据：chunks.jsonl + faiss/
├── scripts/build_faiss_index.py
├── docs/                  # 本文档
└── app/
    ├── api/               # REST + WebSocket
    ├── agents/            # retriever / summarizer / main(qa)
    ├── retrieval/         # faiss / embedder / loader
    └── core/              # session / answer / source
```
