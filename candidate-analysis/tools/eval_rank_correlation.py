#!/usr/bin/env python3
"""Compare static candidate-analysis rankings with dynamic profiler counts."""

from __future__ import annotations

import argparse
import csv
import math
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, Iterable, List, Optional, Tuple

import yaml


ROOT = Path(__file__).resolve().parents[1]
DEFAULT_STATIC_DIR = ROOT / "candidate-analysis-report" / "olden_static"
DEFAULT_DYNAMIC_DIR = ROOT / "candidate-analysis-report" / "olden_dynamic"


FieldTable = Dict[int, float]
StructTable = Dict[str, FieldTable]
ModuleTable = Dict[str, StructTable]


@dataclass
class StructResult:
  module_id: str
  struct_name: str
  field_count: int
  rho: float
  dropped_static: int
  dropped_dynamic: int


def parse_args() -> argparse.Namespace:
  parser = argparse.ArgumentParser(
      description="Compute Spearman rank correlation between static and dynamic reports.",
  )
  parser.add_argument(
      "--static-dir",
      type=Path,
      default=DEFAULT_STATIC_DIR,
      help="Directory that contains type-affinity.yaml files (default: %(default)s)",
  )
  parser.add_argument(
      "--dynamic-dir",
      type=Path,
      default=DEFAULT_DYNAMIC_DIR,
      help="Directory that contains field-access-profiler.yaml files (default: %(default)s)",
  )
  parser.add_argument(
      "--module",
      action="append",
      default=[],
      metavar="NAME",
      help="Restrict to modules whose canonical path contains NAME (can be repeated).",
  )
  parser.add_argument(
      "--struct",
      action="append",
      default=[],
      metavar="NAME",
      help="Restrict to struct names containing NAME (can be repeated).",
  )
  parser.add_argument(
      "--output",
      type=Path,
      help="Optional CSV file to store the per-struct rho results.",
  )
  return parser.parse_args()


def canonical_module_id(raw_label: Optional[str], fallback: str) -> str:
  """Produce a consistent module identifier even if the YAML uses relative paths."""
  label = raw_label or fallback
  path = Path(label)
  # resolve(strict=False) keeps non-existent paths but canonicalizes separators.
  return path.resolve(strict=False).as_posix()


def load_static_reports(root: Path) -> ModuleTable:
  """Map module -> struct -> field index -> static hotness."""
  table: ModuleTable = {}
  for report in sorted(root.rglob("type-affinity.yaml")):
    with report.open("r", encoding="utf-8") as fh:
      data = yaml.safe_load(fh) or {}
    module_id = canonical_module_id(data.get("module"), str(report.parent))
    struct_map = table.setdefault(module_id, {})
    for struct in data.get("structs", []):
      name = struct.get("name")
      if not name:
        continue
      field_map = struct_map.setdefault(name, {})
      for entry in struct.get("field_hotness", []):
        field_map[int(entry["index"])] = float(entry["hotness"])
  return table


def load_dynamic_reports(root: Path) -> ModuleTable:
  """Map module -> struct -> field index -> dynamic counts."""
  table: ModuleTable = {}
  for report in sorted(root.rglob("field-access-profiler.yaml")):
    with report.open("r", encoding="utf-8") as fh:
      data = yaml.safe_load(fh) or {}
    module_id = canonical_module_id(data.get("module"), str(report.parent))
    struct_map = table.setdefault(module_id, {})
    for entry in data.get("fields", []):
      name = entry.get("struct")
      if not name:
        continue
      field_map = struct_map.setdefault(name, {})
      field_map[int(entry["index"])] = float(entry.get("count", 0.0))
  return table


def ranks(values: List[float]) -> List[float]:
  """Assign average ranks to the supplied values."""
  order = sorted(enumerate(values), key=lambda pair: pair[1])
  ranked = [0.0] * len(values)
  i = 0
  while i < len(order):
    j = i + 1
    value = order[i][1]
    while j < len(order) and math.isclose(order[j][1], value, rel_tol=1e-9, abs_tol=1e-9):
      j += 1
    avg_rank = (i + j - 1) / 2.0 + 1.0  # convert zero-based positions to 1-based ranks.
    for k in range(i, j):
      ranked[order[k][0]] = avg_rank
    i = j
  return ranked


def spearman_rho(static_values: List[float], dynamic_values: List[float]) -> float:
  """Compute Spearman's rho by correlating the rank vectors."""
  if len(static_values) != len(dynamic_values):
    raise ValueError("value vectors must match")
  if len(static_values) < 2:
    return float("nan")
  x = ranks(static_values)
  y = ranks(dynamic_values)
  mean_x = sum(x) / len(x)
  mean_y = sum(y) / len(y)
  cov = sum((xi - mean_x) * (yi - mean_y) for xi, yi in zip(x, y))
  var_x = sum((xi - mean_x) ** 2 for xi in x)
  var_y = sum((yi - mean_y) ** 2 for yi in y)
  if var_x == 0 or var_y == 0:
    return float("nan")
  return cov / math.sqrt(var_x * var_y)


def filter_match(value: str, needles: Iterable[str]) -> bool:
  """True if value should pass an optional substring filter."""
  if not needles:
    return True
  return any(needle in value for needle in needles)


def compute_results(
    static_data: ModuleTable,
    dynamic_data: ModuleTable,
    module_filters: Iterable[str],
    struct_filters: Iterable[str],
) -> List[StructResult]:
  results: List[StructResult] = []
  for module_id in sorted(static_data):
    if not filter_match(module_id, module_filters):
      continue
    dyn_structs = dynamic_data.get(module_id)
    if not dyn_structs:
      continue
    for struct_name, static_fields in sorted(static_data[module_id].items()):
      if not filter_match(struct_name, struct_filters):
        continue
      dyn_fields = dyn_structs.get(struct_name)
      if not dyn_fields:
        continue
      shared_indices = sorted(set(static_fields).intersection(dyn_fields))
      if not shared_indices:
        continue
      stat_values = [static_fields[idx] for idx in shared_indices]
      dyn_values = [dyn_fields[idx] for idx in shared_indices]
      rho = spearman_rho(stat_values, dyn_values)
      # count dropped fields for quick debugging.
      dropped_static = len(static_fields) - len(shared_indices)
      dropped_dynamic = len(dyn_fields) - len(shared_indices)
      results.append(
          StructResult(
              module_id=module_id,
              struct_name=struct_name,
              field_count=len(shared_indices),
              rho=rho,
              dropped_static=dropped_static,
              dropped_dynamic=dropped_dynamic,
          )
      )
  return results


def write_csv(results: List[StructResult], path: Path) -> None:
  path.parent.mkdir(parents=True, exist_ok=True)
  with path.open("w", newline="", encoding="utf-8") as fh:
    writer = csv.writer(fh)
    writer.writerow(["module", "struct", "fields", "rho", "dropped_static", "dropped_dynamic"])
    for res in results:
      writer.writerow(
          [res.module_id, res.struct_name, res.field_count, f"{res.rho:.6f}", res.dropped_static, res.dropped_dynamic]
      )


def print_results(results: List[StructResult]) -> None:
  if not results:
    print("No structs matched the provided filters or data directories.", file=sys.stderr)
    return
  header = f"{'Module (basename)':30} {'Struct':25} {'Fields':>6} {'Rho':>8} {'Drop(S)':>8} {'Drop(D)':>8}"
  print(header)
  print("-" * len(header))
  for res in results:
    module_name = Path(res.module_id).name
    rho_text = "nan" if math.isnan(res.rho) else f"{res.rho:8.3f}"
    print(
        f"{module_name:30} {res.struct_name:25} {res.field_count:6d} {rho_text} "
        f"{res.dropped_static:8d} {res.dropped_dynamic:8d}"
    )
  valid_rhos = [res.rho for res in results if math.isfinite(res.rho)]
  if valid_rhos:
    average = sum(valid_rhos) / len(valid_rhos)
    print(f"\nAveraged rho over {len(valid_rhos)} structs: {average:.3f}")
  else:
    print("\nNo valid rho values (insufficient fields or constant rankings).")


def main() -> None:
  args = parse_args()
  static_dir = args.static_dir if args.static_dir.is_absolute() else (Path.cwd() / args.static_dir)
  dynamic_dir = args.dynamic_dir if args.dynamic_dir.is_absolute() else (Path.cwd() / args.dynamic_dir)
  if not static_dir.exists():
    sys.exit(f"Static report directory not found: {static_dir}")
  if not dynamic_dir.exists():
    sys.exit(f"Dynamic report directory not found: {dynamic_dir}")
  static_data = load_static_reports(static_dir)
  dynamic_data = load_dynamic_reports(dynamic_dir)
  results = compute_results(static_data, dynamic_data, args.module, args.struct)
  print_results(results)
  if args.output:
    write_csv(results, args.output)


if __name__ == "__main__":
  main()
