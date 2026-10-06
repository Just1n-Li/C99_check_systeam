"""
Neo4j Tool — 将图数据库查询封装为 LangChain Tool
Graph Agent 通过调用此 Tool 执行 Cypher 语句，获取查询结果。
仅在 ENABLE_NEO4J=on 时才注册给 Agent。

数据来源：graph-db/data/kg/ 下的 8 个节点维表、8 个关系文件、27 个字面量属性文件。
详细 Schema 文档参见：graph-db/docs/graph-structure-for-rag.md
"""

import json
from typing import Optional
from langchain_core.tools import tool
from config import settings

_neo4j_driver = None


NEO4J_SCHEMA_INFO = """
## Neo4j 知识图谱 Schema — 海外藏中国文物

> 数据版本：2026-05-25 ｜ 覆盖博物馆：2 家（史密森尼 + 哈佛）｜ 文物总数：6875
> 所有节点都带基类标签 `Entity`，并以 `uri` 作为唯一键（`CREATE CONSTRAINT entity_uri IF NOT EXISTS FOR (n:Entity) REQUIRE n.uri IS UNIQUE`）。

### 1. 节点（点）—— 11 个标签

#### 1.1 核心 8 类

| 标签 | 含义 | 节点数 | 关键字段 |
| --- | --- | --- | --- |
| `Artifact` | 文物 | 6875 | `uri`, `name`（=title）, `artifact_id`, `museum_id`, `object_id` |
| `Museum` | 博物馆 | 2 | `uri`, `name` |
| `Dynasty` | 朝代 | 21 | `uri`, `name`（带括号，如 `Tang（唐）`） |
| `Artist` | 艺术家/作者 | 465 | `uri`, `name`（部分含 Wikidata Q 号，如 `entity:artist:Q1042125`） |
| `Material` | 材质（规范词） | 37 | `uri`, `name`（如 bronze / silk / porcelain） |
| `ArtifactType` | 文物类型 | 88 | `uri`, `name`（如 Paintings / Ceramics / Sculpture） |
| `Location` | 收藏城市 | 2 | `uri`, `name`（`Washington, DC, USA` / `Cambridge, MA, USA`） |
| `Culture` | 文化/分类标签（英文） | 569 | `uri`, `name`（如 allegory / portraits / women） |

#### 1.2 对齐层 3 类（可选用）

| 标签 | 含义 | 关键字段 |
| --- | --- | --- |
| `EntityMaster` | 跨馆共享实体主表（1184 条） | `canonical_id`, `entity_type`, `label`, `norm_label`, `uri`, `external_id`, `source_count` |
| `EntityAlias` | 实体别名（1244 条） | `alias`, `norm_alias`, `match_method`, `confidence` |
| `EntitySource` | 实体字段溯源（76023 条） | `museum_id`, `object_id`, `field_name`, `raw_value`, `source_file` |

#### 1.3 Artifact 节点上挂载的 27 类字面量属性

- 基础描述：`title`, `description`, `period`, `periodStartYear`, `periodEndYear`
- 材质：`materialSummary`（馆方原文）, `materialBase`（` | ` 拼接的规范词列表）, `materialPrimary`（单值主材质）
- 尺寸/外链/图像：`dimensions`, `detailUrl`, `imageUrl`, `imageUrls`（多图 ` | ` 拼接）, `imagePath`, `imagePaths`, `imageCount`, `iiifManifestUrl`, `accessionNumber`, `creditLine`, `provenance`, `bibliography`, `crawlDate`
- 图像校验（来自 image_check.csv）：`image_http_ok`, `image_status_code`, `image_content_type`, `image_local_file_ok`, `image_valid`, `image_checked_at`
- 作者快照（每件文物一份，多作者时多行）：`artistWikidataId`, `artistBio`, `artistBirth`, `artistDeath`, `artistProvince`, `artistWikipediaSummary`, `artistEnrichedAt`
- 系统字段：`uri`, `updated_at`

### 2. 关系（边）—— 8 条核心 + 2 条对齐

#### 2.1 核心关系（驼峰命名，由 CSV 文件名直接生成）

| 关系名 | 行数 | (主体)-[关系]->(客体) | 含义 |
| --- | --- | --- | --- |
| `belongsToMuseum` | 6875 | (Artifact)-[]->(Museum) | 文物收藏于哪家博物馆 |
| `belongsToDynasty` | 5813 | (Artifact)-[]->(Dynasty) | 文物属于哪个朝代 |
| `createdBy` | 6598 | (Artifact)-[]->(Artist) | 文物由谁创作（一件可多作者） |
| `usesMaterial` | 11884 | (Artifact)-[]->(Material) | 文物使用的所有材质（多值） |
| `hasPrimaryMaterial` | 6501 | (Artifact)-[]->(Material) | 文物主材质（每件最多 1 条） |
| `hasType` | 6875 | (Artifact)-[]->(ArtifactType) | 文物类型 |
| `hasCulture` | 17608 | (Artifact)-[]->(Culture) | 文化/分类标签（多值） |
| `locatedIn` | 6875 | (Artifact)-[]->(Location) | 现藏城市 |

> ⚠️ 关系名是**驼峰**（`belongsToMuseum`、`hasPrimaryMaterial`），不是 `COLLECTED_BY` / `CREATED_BY`，请按上表精确书写。

#### 2.2 对齐关系

| 关系名 | 含义 |
| --- | --- |
| `HAS_ALIAS` | (EntityMaster)-[]->(EntityAlias) |
| `HAS_SOURCE_RECORD` | (EntityMaster)-[]->(EntitySource) |

### 3. URI 约定（**关键**：所有 MATCH 必须用 `uri` 字段定位节点）

| 类型 | URI 模式 | 示例 |
| --- | --- | --- |
| Artifact | `entity:artifact:{museum_id}:{object_id}` | `entity:artifact:1:ld1-1643381040022-1643381041048-0` |
| Museum | `entity:museum:{id}` | `entity:museum:1`（Smithsonian）/ `entity:museum:2`（Harvard） |
| Dynasty | `entity:dynasty:{slug}` | `entity:dynasty:tang唐` |
| Artist | `entity:artist:{slug或QID}` | `entity:artist:Q1042125`（Hua Yan 華喦） |
| Material | `entity:material:{slug}` | `entity:material:porcelain` |
| ArtifactType | `entity:artifacttype:{slug}` | `entity:artifacttype:paintings` |
| Location | `entity:location:{slug}` | `entity:location:washington-dc-usa` |
| Culture | `entity:culture:{slug}` | `entity:culture:portraits` |
| Source/Alias | `entity:source:{sha1}` | 哈希生成 |

> Artifact 节点的标题属性是 **`title`**（不是 `name`）。其余节点（`Museum` / `Dynasty` / `Artist` / `Material` / `ArtifactType` / `Location` / `Culture`）的显示名都是 `name`。

### 4. 业务主键

`(museum_id, object_id)` 是 Web/MySQL 端定位文物的业务键。`data/kg_artifact_map.csv` 保存了 `(museum_id, object_id) ↔ artifact_id(uri)` 的映射。

博物馆编码：`1` = Smithsonian Institution（2244 件）；`2` = Harvard Art Museums（4631 件）；`3` = Museum of Fine Arts, Boston（**尚未入库**）。
"""


NEO4J_USAGE_GUIDE = """
## ⭐ Neo4j 检索使用指南（务必优先阅读）

### 为什么优先使用 Neo4j？

| 场景 | 建议 |
| --- | --- |
| 单条文物详情、模糊搜索、关键词 LIKE | 可用 MySQL（`get_artifact_detail` / `quick_search`） |
| **作者 ↔ 文物 ↔ 朝代 ↔ 材质 ↔ 类型** 多维关系查询 | ✅ **强烈优先用 Neo4j**（关系遍历比 JOIN 快得多） |
| **多跳推理**（如"某作者和某朝代是否有交集""某博物馆某类型有几种主材质"） | ✅ **必须用 Neo4j**（MySQL 多次 JOIN 难以表达） |
| 跨馆同作者/同朝代/同材质归并（实体对齐） | ✅ **必须用 Neo4j**（`EntityMaster` / `EntityAlias` / `HAS_ALIAS`） |
| 字段来源溯源（哪一条字段来自哪个 CSV 哪一行） | ✅ **必须用 Neo4j**（`HAS_SOURCE_RECORD`） |
| 统计聚合（按博物馆/朝代/材质/类型 group by + count） | 两者皆可，Neo4j 更直观 |

> 一句话：**只要问题涉及"关系""遍历""多跳""对齐""溯源"，就走 Neo4j，不要硬塞 MySQL。**

### 标准查询范式

```cypher
// 1) 按朝代找文物
MATCH (a:Artifact)-[:belongsToDynasty]->(d:Dynasty)
WHERE d.name CONTAINS 'Ming'
RETURN a.title AS title, a.period, a.creditLine
LIMIT 50;

// 2) 按作者找作品（含作者信息）
MATCH (a:Artifact)-[:createdBy]->(artist:Artist {uri: 'entity:artist:Q1042125'})
RETURN a.title AS title, a.period, a.artistBio
LIMIT 100;

// 3) 多跳：某博物馆某朝代某材质的文物
MATCH (a:Artifact)-[:belongsToMuseum]->(m:Museum),
      (a)-[:belongsToDynasty]->(d:Dynasty),
      (a)-[:hasPrimaryMaterial]->(mat:Material)
WHERE m.name = 'Harvard Art Museums' AND d.name CONTAINS 'Tang' AND mat.name = 'porcelain'
RETURN a.title AS title, a.creditLine
LIMIT 50;

// 4) 推荐相似：同作者的其他作品
MATCH (a:Artifact {uri: $artifact_uri})-[:createdBy]->(artist:Artist)
MATCH (other:Artifact)-[:createdBy]->(artist)
WHERE other.uri <> a.uri
RETURN other.title AS title, other.uri LIMIT 20;

// 5) 实体对齐：跨馆同作者
MATCH (m:EntityMaster {entity_type: 'Artist'})-[:HAS_ALIAS]->(a:EntityAlias)
WHERE m.label CONTAINS 'Hua Yan'
RETURN m.canonical_id, m.label, collect(a.alias) AS aliases;

// 6) 溯源：某文物 title 字段来自哪条原始记录
MATCH (a:Artifact {uri: $artifact_uri})-[:HAS_SOURCE_RECORD]->(s:EntitySource)
WHERE s.field_name = 'title'
RETURN s.raw_value, s.source_file;
```

### Cypher 书写注意

- 关系名**驼峰**：`belongsToMuseum` / `hasPrimaryMaterial`，不要写 `COLLECTED_BY`。
- 所有节点定位用 `uri`：`MATCH (a:Artifact {uri: 'entity:artifact:1:ld1-1643381040022-1643381041048-0'})`。
- 朝代 `name` 字段带括号：`Tang（唐）`；模糊匹配请用 `CONTAINS 'Tang'`。
- `periodStartYear` / `periodEndYear` 为负数表示 BC（公元前）。
- 多值字段 `imageUrls` / `imagePaths` / `materialBase` 用 ` | `（空格-竖线-空格）分隔。
- **必须加 `LIMIT`**，建议 `LIMIT 100` 以内，避免一次返回过多结果。
- 只读查询，禁止 `MERGE / CREATE / DELETE / SET / DETACH DELETE / CALL ... YIELD`。
"""


ALLOWED_NODE_LABELS = {
    "Artifact", "Museum", "Dynasty", "Artist", "Material",
    "ArtifactType", "Location", "Culture",
    "EntityMaster", "EntityAlias", "EntitySource",
}


def _get_driver():
    global _neo4j_driver
    if _neo4j_driver is None:
        from neo4j import AsyncGraphDatabase
        _neo4j_driver = AsyncGraphDatabase.driver(
            settings.NEO4J_URI,
            auth=(settings.NEO4J_USER, settings.NEO4J_PASSWORD),
            max_connection_pool_size=10,
        )
    return _neo4j_driver


async def close_driver():
    global _neo4j_driver
    if _neo4j_driver:
        await _neo4j_driver.close()
        _neo4j_driver = None


def is_neo4j_enabled() -> bool:
    return settings.ENABLE_NEO4J


def _check_enabled_or_error() -> Optional[str]:
    """如果 Neo4j 未启用，返回 JSON 错误字符串；否则返回 None。"""
    if not settings.ENABLE_NEO4J:
        return json.dumps({"error": "Neo4j 工具未启用（ENABLE_NEO4J=off）"}, ensure_ascii=False)
    return None


def _parse_cypher_params(params: Optional[str]) -> tuple[Optional[dict], Optional[str]]:
    if not params:
        return {}, None
    try:
        return json.loads(params), None
    except json.JSONDecodeError:
        return None, f"参数 JSON 解析失败: {params}"


@tool
async def query_neo4j(cypher: str, params: Optional[str] = None) -> str:
    """
    【首选图数据库检索】执行 Cypher 查询语句，从文物知识图谱中检索数据。

    适用场景（**强烈推荐优先使用**）：
    - 涉及关系/遍历的查询：作者↔文物↔朝代↔材质↔类型之间的关联
    - 多跳推理：如"唐代瓷器在两家博物馆的分布"
    - 跨馆实体对齐：同一作者在不同博物馆的不同写法
    - 字段溯源：某条字段值来自哪条原始记录
    - 统计聚合：按博物馆/朝代/材质 group by

    Args:
        cypher: 合法的 Cypher 查询语句。关系名使用驼峰（`belongsToMuseum` / `hasPrimaryMaterial` 等），
                节点定位优先用 `uri` 字段；必须包含 `LIMIT`。
        params: JSON 格式的查询参数字串，例如 '{"artifact_uri": "entity:artifact:1:abc"}'。

    Returns:
        查询结果的 JSON 字符串；无结果返回 '[]'；出错返回错误信息。
    """
    err = _check_enabled_or_error()
    if err:
        return err

    parsed_params, parse_err = _parse_cypher_params(params)
    if parse_err:
        return json.dumps({"error": parse_err}, ensure_ascii=False)

    cypher_stripped = cypher.strip()
    upper = cypher_stripped.upper()
    forbidden = ("MERGE", "CREATE", "DELETE", "DETACH DELETE", "SET ", "CALL ")
    cypher_tokens = upper.split()
    for kw in forbidden:
        if kw in cypher_tokens or (kw + " ") in upper:
            return json.dumps({"error": f"仅支持只读查询，禁止使用 {kw.strip()}"}, ensure_ascii=False)
    if "LIMIT" not in cypher_tokens:
        return json.dumps({"error": "Cypher 必须包含 LIMIT 子句"}, ensure_ascii=False)

    driver = _get_driver()
    try:
        async with driver.session() as session:
            result = await session.run(cypher, parsed_params or {})
            records = await result.data()
            return json.dumps(records, ensure_ascii=False, default=str)
    except Exception as e:
        return json.dumps({"error": f"Cypher 执行失败: {str(e)}"}, ensure_ascii=False)


@tool
async def get_graph_schema() -> str:
    """
    【必读】获取当前知识图谱的完整 Schema + 使用指南。

    返回内容包括：
    1. 11 个节点标签（Artifact / Museum / Dynasty / Artist / Material / ArtifactType / Location / Culture + 3 个对齐层）
    2. 8 条核心关系 + 2 条对齐关系的名字和方向
    3. 27 类字面量属性
    4. URI 约定和业务主键
    5. 何时优先使用 Neo4j 的指引 + 标准 Cypher 范式

    生成 Cypher 前**必须先调用此工具**了解图谱结构与推荐用法。
    """
    return NEO4J_SCHEMA_INFO + "\n\n" + NEO4J_USAGE_GUIDE


@tool
async def explore_graph_sample(node_label: str) -> str:
    """
    【数据探查】查看某个节点标签的样本数据，了解实际字段格式与值。

    Args:
        node_label: 节点标签，可选：
            Artifact / Museum / Dynasty / Artist / Material / ArtifactType /
            Location / Culture / EntityMaster / EntityAlias / EntitySource

    Returns:
        该类型节点的前 3 条样本数据（JSON）。
    """
    err = _check_enabled_or_error()
    if err:
        return err

    if node_label not in ALLOWED_NODE_LABELS:
        return json.dumps(
            {"error": f"不支持的节点标签: {node_label}，可选: {sorted(ALLOWED_NODE_LABELS)}"},
            ensure_ascii=False,
        )

    cypher = f"MATCH (n:{node_label}) RETURN n LIMIT 3"
    driver = _get_driver()
    try:
        async with driver.session() as session:
            result = await session.run(cypher)
            records = await result.data()
            return json.dumps(records, ensure_ascii=False, default=str)
    except Exception as e:
        return json.dumps({"error": str(e)}, ensure_ascii=False)


@tool
async def count_nodes_by_label(node_label: str) -> str:
    """
    【统计】统计某个节点标签的总数。

    Args:
        node_label: 节点标签（可选同 explore_graph_sample）。

    Returns:
        JSON，例如 {"label": "Artifact", "count": 6875}。
    """
    err = _check_enabled_or_error()
    if err:
        return err

    if node_label not in ALLOWED_NODE_LABELS:
        return json.dumps(
            {"error": f"不支持的节点标签: {node_label}，可选: {sorted(ALLOWED_NODE_LABELS)}"},
            ensure_ascii=False,
        )

    cypher = f"MATCH (n:{node_label}) RETURN count(n) AS count"
    driver = _get_driver()
    try:
        async with driver.session() as session:
            result = await session.run(cypher)
            record = await result.single()
            return json.dumps({"label": node_label, "count": record["count"]}, ensure_ascii=False)
    except Exception as e:
        return json.dumps({"error": str(e)}, ensure_ascii=False)
