#!/usr/bin/env python3
"""
Read candidate-analysis profitability.yaml and emit a pool_alloc_types string.

Usage:
    python profitability_to_pool_list.py <profitability.yaml> [--output pool_alloc_types.txt]

Output (stdout or file):
    TypeA;TypeB;TypeC;

Normalization matches the pool pass rules:
  - strip quotes/whitespace
  - drop trailing pointer markers/spaces
  - remove leading struct./class./union. (or space variants)
  - strip trailing numeric uniquifier (.123)
  - strip trailing .anon
Anonymous structs are skipped (no stable name).
"""
import argparse
import sys
import yaml


def normalize_struct_name(raw: str) -> str:
    """Normalize a struct name the same way the pool pass does."""
    if raw is None:
        return ""
    s = raw.strip()
    if len(s) >= 2 and s[0] == '"' and s[-1] == '"':
        s = s[1:-1]
    s = s.rstrip(" *")

    for prefix in ("struct.", "class.", "union.", "struct ", "class ", "union "):
        if s.startswith(prefix):
            s = s[len(prefix) :]
            break

    dot = s.rfind(".")
    if dot != -1 and dot + 1 < len(s) and s[dot + 1 :].isdigit():
        s = s[:dot]

    if s.endswith(".anon"):
        s = s[: -len(".anon")]

    return s


def extract_candidates(path: str):
    with open(path, "r") as f:
        data = yaml.safe_load(f)
    candidates = []
    structs = data.get("structs", []) if isinstance(data, dict) else []
    for entry in structs:
        if not isinstance(entry, dict):
            continue
        name = normalize_struct_name(entry.get("name", ""))
        if not name:
            continue
        candidate_flag = entry.get("candidate", False)
        if candidate_flag:
            candidates.append(name)
    return candidates


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("profitability", help="path to profitability.yaml")
    ap.add_argument("--output", help="optional output file; defaults to stdout")
    args = ap.parse_args()

    names = extract_candidates(args.profitability)
    out_str = ";".join(names) + (";" if names else "")

    if args.output:
        with open(args.output, "w") as f:
            f.write(out_str)
    else:
        sys.stdout.write(out_str)


if __name__ == "__main__":
    main()
