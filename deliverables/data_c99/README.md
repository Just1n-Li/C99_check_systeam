# C99 条款数据（§6.1–§7.26 全标准）—— 数据留痕

> 本目录记录「C99 编译器测试代码生成」项目的数据准备过程，供答辩汇报溯源。

## 数据来源

- **标准**：ISO/IEC 9899:1999（C99）+ 技术勘误 TC1/TC2/TC3（即 **N1256**，2007-09-07 版本）
- **原始文本**：N1256 的 HTML 渲染版，来源 https://port70.net/~nsz/c/c99/n1256.html
- **本地留存**：`00_raw/n1256_c99_tc3.html`（原始 HTML，未做任何修改）

## 切分范围（全标准）

已扩展到 **§6.1–§7.26 全部条款，共 581 条**：

- **第 6 章 语言（131 条）**：6.1 记号约定、6.2 概念（作用域/链接/类型等）、6.3 转换、6.4 词法元素、6.5 表达式、6.6 常量表达式、6.7 声明、6.8 语句与块、6.9 外部定义、6.10 预处理指令、6.11 未来语言方向。
- **第 7 章 库（450 条）**：7.1 引言，7.2~7.25 各标准库头（`<assert.h>`…`<wctype.h>`），含每个库函数子条款（如 7.19.6.3 `printf`、7.20.3.3 `malloc`、7.21.2.3 `strcpy`），7.26 未来库方向。
- 嵌套最深到 4 层（如 `7.24.4.1.1` wcstod 系列），HTML 只用 h3~h5，正则 `<h[3-5]>` 均能覆盖。

## 切分方法

- 脚本：`01_split/split_c99.py`（可重跑）
- 原理：解析 HTML 中 `<h3/h4/h5 name="6.5.x">` 条款标题锚点与 `<p name="...pN">` 段落锚点，
  按「每个带独立编号的条款 = 一条 chunk」切分，正文保留段落编号、`Constraints`/`Semantics`/`Footnotes`/`EXAMPLE` 结构。
- 去除了：脚注上标引用、每页末尾的「返回目录」导航、HTML 标签与实体。

## 输出格式（`chunks.jsonl`）

`chunks.jsonl`，每行一条：

```json
{
  "id": "c99-6.5.2.2",
  "text": "Function calls\nConstraints:\n[1] ...",
  "law_name": "ISO/IEC 9899:1999 (C99)",
  "article": "6.5.2.2",
  "status": null,
  "source_url": "https://port70.net/~nsz/c/c99/n1256.html#6.5.2.2",
  "source_file": "n1256_c99_tc3.html"
}
```

字段映射到系统内部语义：

| 字段名 | C99 语义 |
|-----------|---------|
| `law_name` | 标准名（ISO/IEC 9899:1999 (C99)） |
| `article`  | 条款编号（如 `6.5.2.2`） |
| `text`     | 条款标题 + 正文 |
| `id`       | `c99-<条款编号>` |

## 向量化与索引

- 脚本：`../scripts/build_faiss_index.py`（读取 `chunks.jsonl`，写入 `faiss/`）
- 向量化方案：sentence-transformers `paraphrase-multilingual-MiniLM-L12-v2`（dim 384），支持中文描述 → 英文条款跨语言检索
- 索引：`faiss/index.faiss` + `faiss/meta.json` + `faiss/embed_info.json`（当前 581 向量）
