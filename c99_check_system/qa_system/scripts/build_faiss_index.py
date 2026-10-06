#!/usr/bin/env python3
"""
scripts/build_faiss_index.py — 从清洗交付数据构建 FAISS 索引

默认读取清洗同学交付目录：
  data_cleaned/chunks.jsonl
  data_cleaned/laws.json（可选，用于补全 source_url / status）

用法（在 qa_system 目录下）:
  python scripts/build_faiss_index.py
  python scripts/build_faiss_index.py --input data_cleaned/chunks.jsonl --laws data_cleaned/laws.json
  python scripts/build_faiss_index.py --input data/legal/sample_chunks.json   # 旧样例 JSON 数组也可

输出:
  data/faiss/index.faiss
  data/faiss/meta.json
  data/faiss/embed_info.json
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))


def main() -> None:
    parser = argparse.ArgumentParser(description="Build FAISS index from cleaned legal chunks")
    parser.add_argument(
        "--input",
        type=str,
        default=None,
        help="chunks.jsonl 或 chunks JSON 数组路径（默认: LAW_DATA_DIR/chunks.jsonl）",
    )
    parser.add_argument(
        "--laws",
        type=str,
        default=None,
        help="laws.json 路径（默认: 与 chunks 同目录下的 laws.json）",
    )
    parser.add_argument(
        "--output",
        type=str,
        default=None,
        help="FAISS 输出目录（默认: FAISS_INDEX_DIR）",
    )
    parser.add_argument("--model", type=str, default=None)
    args = parser.parse_args()

    from config import settings, BASE_DIR
    from app.retrieval.embedder import TextEmbedder
    from app.retrieval.legal_data_loader import load_legal_chunks

    cleaned_dir = Path(settings.LAW_DATA_DIR)
    if not cleaned_dir.is_absolute():
        cleaned_dir = (BASE_DIR / cleaned_dir).resolve()

    if args.input:
        input_path = Path(args.input)
        if not input_path.is_absolute():
            for cand in (Path.cwd() / input_path, BASE_DIR / input_path, ROOT / input_path):
                if cand.exists():
                    input_path = cand.resolve()
                    break
            else:
                input_path = (BASE_DIR / input_path).resolve()
    else:
        input_path = (cleaned_dir / "chunks.jsonl").resolve()

    if args.laws:
        laws_path = Path(args.laws)
        if not laws_path.is_absolute():
            for cand in (Path.cwd() / laws_path, BASE_DIR / laws_path, ROOT / laws_path):
                if cand.exists():
                    laws_path = cand.resolve()
                    break
            else:
                laws_path = (BASE_DIR / laws_path).resolve()
    else:
        laws_path = input_path.parent / "laws.json"

    output_dir = Path(args.output) if args.output else Path(settings.FAISS_INDEX_DIR)
    if not output_dir.is_absolute():
        output_dir = BASE_DIR / output_dir
    output_dir.mkdir(parents=True, exist_ok=True)

    model_name = args.model or settings.EMBEDDING_MODEL

    print(f"Input chunks: {input_path}")
    print(f"Laws meta:    {laws_path} ({'exists' if laws_path.exists() else 'missing'})")
    print(f"Output dir:   {output_dir}")

    chunks = load_legal_chunks(input_path, laws_path if laws_path.exists() else None)
    if not chunks:
        raise SystemExit(f"No valid chunks loaded from {input_path}")

    texts = []
    meta = []
    for c in chunks:
        law = c.get("law_name") or ""
        article = c.get("article") or ""
        text = c.get("text") or ""
        # 拼接标题提升检索命中率
        embed_text = f"{law} {article}：{text}" if law or article else text
        texts.append(embed_text)
        meta.append({
            "chunk_id": c["chunk_id"],
            "law_id": c.get("law_id") or "",
            "law_name": law,
            "article": article,
            "text": text,
            "source": c.get("source") or "",
            "status": c.get("status"),
            "source_file": c.get("source_file") or "",
        })

    embedder = TextEmbedder(model_name)
    backend = embedder.load()
    print(f"Embedding backend: {backend}, dim={embedder.dim}")

    vectors = embedder.encode(texts)

    try:
        import faiss
    except ImportError:
        raise SystemExit(
            "faiss-cpu is required. Install with: pip install faiss-cpu\n"
            "If Windows path is too long, enable Win32 long paths or use a shorter venv path."
        )

    import numpy as np

    vectors = np.asarray(vectors, dtype="float32")
    index = faiss.IndexFlatIP(embedder.dim)
    index.add(vectors)

    index_path = output_dir / "index.faiss"
    meta_path = output_dir / "meta.json"
    info_path = output_dir / "embed_info.json"

    enc = faiss.serialize_index(index)
    index_path.write_bytes(np.asarray(enc, dtype="uint8").tobytes())
    with open(meta_path, "w", encoding="utf-8") as f:
        json.dump(meta, f, ensure_ascii=False, indent=2)
    with open(info_path, "w", encoding="utf-8") as f:
        json.dump(
            {
                "backend": backend,
                "model": model_name,
                "dim": embedder.dim,
                "count": len(meta),
                "source_chunks": str(input_path),
                "source_laws": str(laws_path) if laws_path.exists() else None,
            },
            f,
            ensure_ascii=False,
            indent=2,
        )

    print(f"Wrote {len(meta)} vectors (dim={embedder.dim}) -> {index_path}")
    print(f"Wrote metadata -> {meta_path}")
    print(f"Wrote embed info -> {info_path}")


if __name__ == "__main__":
    main()
