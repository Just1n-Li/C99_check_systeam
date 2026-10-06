"""
app/db/mysql_client.py — MySQL 数据库客户端

职责：
  从 MySQL 中查询文物信息，替代 Mock 数据，提供真实的文物数据。
  同时支持溯源信息查询（detail_url 等）。

MySQL 表结构（artifact 表，35列）：
  联合主键：(object_id, museum_id)
  核心字段：title, artist, dynasty, period, type, material, description, dimensions,
            museum, location, detail_url, image_url, accession_number ...
"""

import aiomysql
from typing import Optional

from config import settings


async def get_pool():
    return await MySQLClient.get_pool()


class MySQLClient:

    _pool = None

    @classmethod
    async def get_pool(cls):
        if cls._pool is None:
            cls._pool = await aiomysql.create_pool(
                host=settings.MYSQL_HOST,
                port=settings.MYSQL_PORT,
                user=settings.MYSQL_USER,
                password=settings.MYSQL_PASSWORD,
                db=settings.MYSQL_DB,
                charset="utf8mb4",
                autocommit=True,
            )
        return cls._pool

    @classmethod
    async def close_pool(cls):
        if cls._pool:
            cls._pool.close()
            await cls._pool.wait_closed()
            cls._pool = None

    @classmethod
    async def execute_query(cls, sql: str, params: tuple = ()) -> list:
        pool = await cls.get_pool()
        async with pool.acquire() as conn:
            async with conn.cursor(aiomysql.DictCursor) as cur:
                await cur.execute(sql, params)
                return await cur.fetchall()

    @classmethod
    async def get_artifact_detail(cls, object_id: str) -> dict:
        pool = await cls.get_pool()
        async with pool.acquire() as conn:
            async with conn.cursor(aiomysql.DictCursor) as cur:
                await cur.execute(
                    """SELECT object_id, title, type, material, dynasty, period,
                              description, dimensions, museum, location,
                              detail_url, image_url, accession_number
                       FROM artifact WHERE object_id = %s""",
                    (object_id,)
                )
                return await cur.fetchone() or {}

    @classmethod
    async def get_artifacts_by_artifact_ids(cls, artifact_ids: list[str]) -> list:
        if not artifact_ids:
            return []
        ids = artifact_ids[:50]
        placeholders = ", ".join(["%s"] * len(ids))
        pool = await cls.get_pool()
        async with pool.acquire() as conn:
            async with conn.cursor(aiomysql.DictCursor) as cur:
                await cur.execute(
                    f"""SELECT object_id, museum_id, artifact_id, title, museum,
                               detail_url, image_url, accession_number
                        FROM artifact
                        WHERE artifact_id IN ({placeholders})
                          AND detail_url IS NOT NULL AND detail_url != ''""",
                    tuple(ids),
                )
                return await cur.fetchall()

    @classmethod
    async def get_artifacts_by_titles(
        cls,
        title_museum_pairs: list[tuple[str, int]],
        limit: int = 30,
    ) -> list:
        """按标题 + museum_id 精确匹配，从 MySQL 补全 detail_url（每对最多 1 条）。"""
        if not title_museum_pairs:
            return []
        pool = await cls.get_pool()
        results: list[dict] = []
        seen: set[str] = set()

        async with pool.acquire() as conn:
            async with conn.cursor(aiomysql.DictCursor) as cur:
                for title, museum_id in title_museum_pairs:
                    if len(results) >= limit:
                        break
                    await cur.execute(
                        """SELECT object_id, museum_id, artifact_id, title, museum,
                                  detail_url, image_url, accession_number
                           FROM artifact
                           WHERE title = %s AND museum_id = %s
                             AND detail_url IS NOT NULL AND detail_url != ''
                           LIMIT 1""",
                        (title, museum_id),
                    )
                    for row in await cur.fetchall():
                        key = row.get("object_id") or ""
                        if key and key not in seen:
                            seen.add(key)
                            results.append(row)
        return results

    @classmethod
    async def search_by_title(cls, entity: str, limit: int = 5) -> list:
        pool = await cls.get_pool()
        async with pool.acquire() as conn:
            async with conn.cursor(aiomysql.DictCursor) as cur:
                await cur.execute(
                    """SELECT object_id, title, artist, dynasty, period,
                              type, material, culture, description, dimensions,
                              museum, location, detail_url, image_url, accession_number,
                              artist_bio, artist_birth, artist_death, artist_province
                       FROM artifact
                       WHERE title LIKE %s
                       LIMIT %s""",
                    (f"%{entity}%", limit)
                )
                return await cur.fetchall()

    @classmethod
    async def search_by_artist(cls, entity: str, limit: int = 10) -> list:
        pool = await cls.get_pool()
        async with pool.acquire() as conn:
            async with conn.cursor(aiomysql.DictCursor) as cur:
                await cur.execute(
                    """SELECT object_id, title, artist, dynasty, period,
                              type, material, description, dimensions,
                              museum, location, detail_url, image_url, accession_number,
                              artist_bio, artist_birth, artist_death, artist_province
                       FROM artifact
                       WHERE artist LIKE %s
                       LIMIT %s""",
                    (f"%{entity}%", limit)
                )
                return await cur.fetchall()

    @classmethod
    async def search_by_dynasty(cls, entity: str, limit: int = 10) -> list:
        pool = await cls.get_pool()
        async with pool.acquire() as conn:
            async with conn.cursor(aiomysql.DictCursor) as cur:
                await cur.execute(
                    """SELECT object_id, title, artist, dynasty, period,
                              type, material, description, dimensions,
                              museum, location, detail_url, image_url, accession_number
                       FROM artifact
                       WHERE dynasty LIKE %s
                       LIMIT %s""",
                    (f"%{entity}%", limit)
                )
                return await cur.fetchall()

    @classmethod
    async def search_by_museum(cls, entity: str, limit: int = 10) -> list:
        pool = await cls.get_pool()
        async with pool.acquire() as conn:
            async with conn.cursor(aiomysql.DictCursor) as cur:
                await cur.execute(
                    """SELECT museum, location, COUNT(*) AS artifact_count
                       FROM artifact
                       WHERE museum LIKE %s
                       GROUP BY museum, location
                       LIMIT %s""",
                    (f"%{entity}%", limit)
                )
                return await cur.fetchall()

    @classmethod
    async def get_artifacts_by_object_ids(cls, object_ids: list[str], limit: int = 30) -> list:
        if not object_ids:
            return []
        ids = object_ids[:limit]
        placeholders = ", ".join(["%s"] * len(ids))
        pool = await cls.get_pool()
        async with pool.acquire() as conn:
            async with conn.cursor(aiomysql.DictCursor) as cur:
                await cur.execute(
                    f"""SELECT object_id, title, museum, detail_url, image_url, accession_number
                        FROM artifact
                        WHERE object_id IN ({placeholders})
                          AND detail_url IS NOT NULL AND detail_url != ''
                        LIMIT %s""",
                    (*ids, limit),
                )
                return await cur.fetchall()

    @classmethod
    async def get_artifacts_by_title_names(cls, titles: list[str], limit: int = 20) -> list:
        if not titles:
            return []
        pool = await cls.get_pool()
        results: list[dict] = []
        seen: set[str] = set()
        async with pool.acquire() as conn:
            async with conn.cursor(aiomysql.DictCursor) as cur:
                for title in titles[:10]:
                    if len(results) >= limit:
                        break
                    await cur.execute(
                        """SELECT object_id, title, museum, detail_url, image_url, accession_number
                           FROM artifact
                           WHERE title = %s
                             AND detail_url IS NOT NULL AND detail_url != ''
                           LIMIT 2""",
                        (title,),
                    )
                    for row in await cur.fetchall():
                        key = row.get("object_id") or row.get("detail_url") or ""
                        if key and key not in seen:
                            seen.add(key)
                            results.append(row)
        return results

    @classmethod
    async def sample_artifacts_by_field_values(
        cls,
        field: str,
        values: list[str],
        limit: int = 8,
    ) -> list:
        allowed = {"museum", "dynasty", "type", "location", "artist"}
        if field not in allowed or not values:
            return []
        values = [v.strip() for v in values if v and str(v).strip()][:5]
        if not values:
            return []
        clauses = " OR ".join([f"`{field}` LIKE %s"] * len(values))
        params = [f"%{v}%" for v in values] + [limit]
        pool = await cls.get_pool()
        async with pool.acquire() as conn:
            async with conn.cursor(aiomysql.DictCursor) as cur:
                await cur.execute(
                    f"""SELECT object_id, title, museum, detail_url, image_url, accession_number
                        FROM artifact
                        WHERE detail_url IS NOT NULL AND detail_url != ''
                          AND ({clauses})
                        ORDER BY museum, object_id
                        LIMIT %s""",
                    tuple(params),
                )
                return await cur.fetchall()

    @classmethod
    async def sample_distribution_sources(
        cls,
        *,
        dynasty: str,
        artifact_type: str | None = None,
        limit: int = 8,
    ) -> list:
        if not dynasty.strip():
            return []
        conditions = [
            "detail_url IS NOT NULL AND detail_url != ''",
            "dynasty LIKE %s",
        ]
        params: list = [f"%{dynasty.strip()}%"]
        if artifact_type and artifact_type.strip():
            conditions.append("(type LIKE %s OR material LIKE %s OR type LIKE %s)")
            t = artifact_type.strip()
            params.extend([f"%{t}%", f"%{t}%", "%Ceramic%"])
        params.append(limit)
        pool = await cls.get_pool()
        async with pool.acquire() as conn:
            async with conn.cursor(aiomysql.DictCursor) as cur:
                await cur.execute(
                    f"""SELECT object_id, title, museum, detail_url, image_url, accession_number
                        FROM artifact
                        WHERE {' AND '.join(conditions)}
                        ORDER BY museum, object_id
                        LIMIT %s""",
                    tuple(params),
                )
                return await cur.fetchall()

    @classmethod
    async def sample_artifacts_with_url(
        cls,
        field: str,
        value: str,
        limit: int = 2,
    ) -> list:
        """统计/聚合类查询无 detail_url 时，按维度抽样可溯源的代表文物。"""
        allowed = {"museum", "dynasty", "type", "location", "artist"}
        if field not in allowed or not value.strip():
            return []
        pool = await cls.get_pool()
        async with pool.acquire() as conn:
            async with conn.cursor(aiomysql.DictCursor) as cur:
                await cur.execute(
                    f"""SELECT object_id, title, museum, detail_url, image_url, accession_number
                        FROM artifact
                        WHERE `{field}` LIKE %s
                          AND detail_url IS NOT NULL AND detail_url != ''
                        ORDER BY object_id
                        LIMIT %s""",
                    (f"%{value.strip()}%", limit),
                )
                return await cur.fetchall()

    @classmethod
    async def get_similar_artifacts(
        cls, artifact_type: str, material: str, dynasty: str,
        exclude_object_id: str, limit: int = 10
    ) -> list:
        pool = await cls.get_pool()
        async with pool.acquire() as conn:
            async with conn.cursor(aiomysql.DictCursor) as cur:
                await cur.execute(
                    """SELECT object_id, title, type, material, dynasty,
                              museum, location, detail_url, image_url, accession_number,
                              CASE
                                WHEN type = %s AND material LIKE %s THEN '类型+材质相同'
                                WHEN type = %s AND dynasty LIKE %s THEN '类型+朝代相同'
                                ELSE '类型相同'
                              END AS match_reason
                       FROM artifact
                       WHERE object_id != %s AND type = %s
                       ORDER BY
                         CASE
                           WHEN type = %s AND material LIKE %s THEN 1
                           WHEN type = %s AND dynasty LIKE %s THEN 2
                           ELSE 3
                         END
                       LIMIT %s""",
                    (
                        artifact_type, f"%{material}%",
                        artifact_type, f"%{dynasty}%",
                        exclude_object_id, artifact_type,
                        artifact_type, f"%{material}%",
                        artifact_type, f"%{dynasty}%",
                        limit,
                    )
                )
                return await cur.fetchall()
