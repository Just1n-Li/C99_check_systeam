# Agent 与 Tool 设计

## 1. 三 Agent 分工

| 角色 | 实现 | 职责 |
|------|------|------|
| 检索 Agent | `retriever_agent.py` | 编号精确匹配 / 关键词映射 / 调 FAISS |
| 总结 Agent | `summarizer_agent.py` | 将 hits 整理为 RAG context + sources |
| 生成 Agent | `main_agent.py` | 多轮历史 + 条款上下文 → 流式生成 C 测试代码 |

`graph_agent.py` 为兼容入口：内部顺序调用检索 + 总结，供 SessionManager / 旧代码使用。

```text
run_main_agent (QA)
  → run_graph_agent
       → run_retriever_agent
       → run_summarizer_agent
  → LLM stream 生成 C 测试代码
```

---

## 2. FAISS Tool

`app/agents/tools/faiss_tool.py`：

| Tool | 参数 | 返回 |
|------|------|------|
| `search_legal_docs` | `query`, `top_k?` | JSON：`hits[]`（chunk_id / law_name / article / text / source / score） |

> 工具名沿用历史命名 `search_legal_docs`，语义为「检索 C99 条款」。

底层：`FaissRetriever.search()`，索引来自 `data_c99/faiss/`。

---

## 3. Retriever Agent

按优先级：

1. **编号精确匹配**：`ARTICLE_RE` 命中纯条款编号（如 `6.5.2.2`）→ `get_by_article` 精确匹配，`intent_label=EXACT_ARTICLE`
2. **关键词映射**：命中 `C99_KEYWORD_ARTICLES` 中的中文关键词 → 直接定位条款，`intent_label=KEYWORD_MATCH`
3. **语义检索**：调用 `search_legal_docs` → `intent_label=LEGAL_RETRIEVE`
4. 均无结果 → `intent_label=NOT_FOUND`

有对话历史时，可选 LLM 改写检索查询（指代消解）。

---

## 4. Summarizer Agent

1. `format_hits_as_context`：确定性拼接条款正文
2. 可选 LLM 再压缩
3. `chunks_to_sources`：映射为前端兼容 sources

无命中时返回空 context，`has_kg_facts=false`。

---

## 5. QA Agent System Prompt（要点）

- 角色：C99 测试代码生成器
- 严格依据检索到的条款，生成可编译的 C 测试程序
- 正向测试覆盖语义（`assert` 断言 + 可运行），负向测试覆盖约束（`#if 0` 包裹违规片段）
- 注释标注每个验证点（`/* [验证点 i/N] ... */`）与预期行为
- 支持多轮指代

无检索结果时直接返回「未检索到对应条款」，不凭空生成。

---

## 6. 遗留 Tool

`mysql_tool` / `neo4j_tool` 为旧课设遗留，**默认不注册进当前 Agent 工具列表**。
