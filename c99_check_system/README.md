# 面向编译器的测试用例理解与测试规约关联匹配研究

> 以 C99 标准条款为测试规约，自动生成编译器符合性测试代码，并量化评估生成代码对条款的覆盖率。

## 项目简介

本项目面向「测试规约（Test Specification）→ 测试用例（Test Case）」的自动化生成问题，选取 **ISO/IEC 9899:1999（C99，含 TC1/TC2/TC3，即 N1256）** 为标准对象：

- **测试规约理解**：解析 C99 标准全部条款（§6.1–§7.26，共 581 条），结构化保留 `Constraints` / `Semantics` / `Footnotes` / `EXAMPLE` 等规范语义。
- **关联匹配**：支持「条款编号精确匹配」「中文描述跨语言语义检索」「中文关键词确定性映射」三种方式，把自然语言查询定位到对应条款。
- **测试代码生成**：针对每条条款生成**正向测试**（语义 → 可编译运行 + `assert` 断言）与**负向测试**（约束 → `#if 0` 包裹的违规片段，预期编译报错），注释标注每个验证点与预期行为。

## 覆盖率评估结果

| 章节 | 范围 | 条款数 | 平均覆盖率 | 满分条款 |
|---|---|---|---|---|
| 第 6 章 语言 | 6.1–6.11 | 112 | **97.9%** | 93 |
| 第 7 章 库 | 7.1–7.25 | 384 | **93.6%** | 73 |
| **合计** | — | **496** | **94.6%** | **166** |

最终热力图与逐条结果见 [`deliverables/coverage/`](../deliverables/coverage/)（`heatmap_combined.png` 总览 + `ch6/` + `ch7/` 分章结果与生成的 `.c` 测试代码）。

## 系统架构

```
用户 / 前端（Vue3 + Vite）
  → FastAPI（REST + WebSocket 流式）
  → QA Agent（生成测试代码）
       → Retriever Agent（FAISS 检索 C99 条款）
       → Summarizer Agent（整理条款与溯源）
  → LLM（DeepSeek / OpenAI 兼容 API）
```

技术选型：**FAISS + sentence-transformers**（`paraphrase-multilingual-MiniLM-L12-v2`，dim 384，支持中文→英文跨语言检索）+ FastAPI + Vue3。

## 目录结构

```text
C99_check_system/                 # 仓库根
├── c99_check_system/             # 源码（本 README 所在）
│   ├── chat-web/                 # 前端（Vue3 + Vite，C 语法高亮、流式展示）
│   └── qa_system/                # 后端（FastAPI + RAG + Agent）
│       ├── data_c99/             # C99 全标准条款数据（chunks.jsonl + FAISS 索引）
│       ├── scripts/              # 建索引 / 覆盖率生成脚本
│       ├── docs/                 # 设计文档
│       ├── config.py / main.py   # 配置与入口
│       └── .env.example          # 环境变量模板
└── deliverables/                 # 成果与留痕（与源码分离）
    ├── coverage/                 # 覆盖率成果（热力图 + results.json + 生成的 .c）
    └── data_c99/                 # 数据留痕快照
```

## 数据管线

1. **原始数据**：N1256 HTML（`data_c99/00_raw/n1256_c99_tc3.html`）。
2. **切分**：`data_c99/01_split/split_c99.py` 按「带独立编号的条款 = 一条 chunk」切分，保留段落编号与 Constraints/Semantics 结构 → `data_c99/chunks.jsonl`（581 条）。
3. **向量化与索引**：`scripts/build_faiss_index.py` 生成 `data_c99/faiss/`（581 向量）。
4. **检索**：编号精确匹配 + 语义检索 + 关键词确定性映射（`app/agents`）。
5. **生成**：`app/retrieval/llm_generator.py` 输出正/负向测试代码。
6. **覆盖率**：`scripts/gen_coverage.py`（并发生成）+ `scripts/retry_failed.py`（失败重试），结果写入 `deliverables/coverage/`。

## 快速开始

### 后端

```powershell
cd c99_check_system/qa_system
# 1) 配置：复制 .env.example 为 .env，填写 LLM 地址/密钥/模型
# 2) 安装依赖
python -m venv .venv; .\.venv\Scripts\Activate.ps1; pip install -r requirements.txt
# 3) 建索引（如未生成）
python scripts/build_faiss_index.py
# 4) 启动
python -m uvicorn main:app --host 0.0.0.0 --port 8000
```

### 前端

```powershell
cd c99_check_system/chat-web
npm install
npm run dev -- --port 5173
```

## 接口

| 方法 | 路径 | 说明 |
|---|---|---|
| POST | `/api/qa/session` | 创建会话 |
| POST | `/api/qa/ask` | 非流式生成（返回 C 测试代码） |
| WebSocket | `/api/qa/ws?session_id=...` | 流式生成（主交互） |

## 模型配置

在 `.env` 中配置 OpenAI 兼容接口即可，推荐：

```env
USE_OLLAMA=off
LLM_BASE_URL=https://api.deepseek.com/v1
LLM_MODEL_NAME=deepseek-chat
LLM_API_KEY=你的密钥
```

> 深度推理类模型（如部分 SenseNova 型号）易出现思考过程占用 token 上限导致空正文，建议使用非推理模型（如 `deepseek-chat`）。
