#!/usr/bin/env python3
"""Evaluate how well static cold predictions match dynamic cold reality."""

from __future__ import annotations

import argparse
import csv
import math
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, Iterable, List, Optional

import yaml


ROOT = Path(__file__).resolve().parents[1]
DEFAULT_STATIC_DIR = ROOT / "candidate-analysis-report" / "olden_static"
DEFAULT_DYNAMIC_DIR = ROOT / "candidate-analysis-report" / "olden_dynamic"
DEFAULT_THRESHOLD = 7.5


FieldTable = Dict[int, float]
StructTable = Dict[str, FieldTable]
ModuleTable = Dict[str, StructTable]


@dataclass
class ConfusionResult:
  module_id: str
  struct_name: str
  field_count: int
  true_positive: int
  false_positive: int
  false_negative: int
  true_negative: int
  precision: float
  recall: float


def parse_args() -> argparse.Namespace:
  parser = argparse.ArgumentParser(
      description="Compare static cold-field predictions against dynamic ground truth.",
  )
  parser.add_argument(
      "--static-dir",
      type=Path,
      default=DEFAULT_STATIC_DIR,
      help="Directory containing type-affinity.yaml files (default: %(default)s)",
  )
  parser.add_argument(
      "--dynamic-dir",
      type=Path,
      default=DEFAULT_DYNAMIC_DIR,
      help="Directory containing field-access-profiler.yaml files (default: %(default)s)",
  )
  parser.add_argument(
      "--module",
      action="append",
      default=[],
      metavar="NAME",
      help="Restrict to modules whose canonical path contains NAME (repeatable).",
  )
  parser.add_argument(
      "--struct",
      action="append",
      default=[],
      metavar="NAME",
      help="Restrict to struct names containing NAME (repeatable).",
  )
  parser.add_argument(
      "--threshold",
      type=float,
      default=DEFAULT_THRESHOLD,
      help="Cold-field threshold as a percentage (default: %(default)s).",
  )
  parser.add_argument(
      "--output",
      type=Path,
      help="Optional CSV path to store the per-struct confusion results.",
  )
  return parser.parse_args()


def canonical_module_id(raw_label: Optional[str], fallback: str) -> str:
  """Normalize module identifiers so static and dynamic files match."""
  label = raw_label or fallback
  path = Path(label)
  return path.resolve(strict=False).as_posix()


def load_static_reports(root: Path) -> ModuleTable:
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


def relative_percentages(field_map: FieldTable) -> Dict[int, float]:
  """Return each field's relative hotness scaled to 0-100 within its struct."""
  if not field_map:
    return {}
  max_value = max(field_map.values())
  if max_value <= 0:
    return {idx: 0.0 for idx in field_map}
  return {idx: (value / max_value) * 100.0 for idx, value in field_map.items()}


def filter_match(value: str, needles: Iterable[str]) -> bool:
  if not needles:
    return True
  return any(needle in value for needle in needles)


def classify_cold(percent: float, threshold: float) -> bool:
  """True when the relative hotness percentage is below the cold threshold."""
  return percent < threshold


def compute_results(
    static_data: ModuleTable,
    dynamic_data: ModuleTable,
    threshold: float,
    module_filters: Iterable[str],
    struct_filters: Iterable[str],
) -> List[ConfusionResult]:
  results: List[ConfusionResult] = []
  for module_id in sorted(static_data):
    if not filter_match(module_id, module_filters):
      continue
    dyn_structs = dynamic_data.get(module_id, {})
    for struct_name, static_fields in sorted(static_data[module_id].items()):
      if not filter_match(struct_name, struct_filters):
        continue
      dyn_fields = dyn_structs.get(struct_name, {})
      static_pct = relative_percentages(static_fields)
      # Treat unseen dynamic fields as zero-count (cold) by default.
      dyn_pct = relative_percentages(dyn_fields)
      indices = sorted(static_fields.keys())
      if not indices:
        continue
      tp = fp = fn = tn = 0
      matched = 0
      for idx in indices:
        dyn_value = dyn_pct.get(idx, 0.0)
        static_value = static_pct.get(idx, 0.0)
        static_cold = classify_cold(static_value, threshold)
        dynamic_cold = classify_cold(dyn_value, threshold)
        matched += 1
        if static_cold and dynamic_cold:
          tp += 1
        elif static_cold and not dynamic_cold:
          fp += 1
        elif not static_cold and dynamic_cold:
          fn += 1
        else:
          tn += 1
      precision = float("nan") if (tp + fp) == 0 else tp / (tp + fp)
      recall = float("nan") if (tp + fn) == 0 else tp / (tp + fn)
      results.append(
          ConfusionResult(
              module_id=module_id,
              struct_name=struct_name,
              field_count=matched,
              true_positive=tp,
              false_positive=fp,
              false_negative=fn,
              true_negative=tn,
              precision=precision,
              recall=recall,
          )
      )
  return results


def write_csv(results: List[ConfusionResult], path: Path) -> None:
  path.parent.mkdir(parents=True, exist_ok=True)
  with path.open("w", newline="", encoding="utf-8") as fh:
    writer = csv.writer(fh)
    writer.writerow(
        [
            "module",
            "struct",
            "fields",
            "tp",
            "fp",
            "fn",
            "tn",
            "precision",
            "recall",
        ]
    )
    for res in results:
      writer.writerow(
          [
              res.module_id,
              res.struct_name,
              res.field_count,
              res.true_positive,
              res.false_positive,
              res.false_negative,
              res.true_negative,
              f"{res.precision:.6f}" if math.isfinite(res.precision) else "nan",
              f"{res.recall:.6f}" if math.isfinite(res.recall) else "nan",
          ]
      )


def print_results(results: List[ConfusionResult]) -> None:
  if not results:
    print("No structs matched the provided filters or data directories.", file=sys.stderr)
    return
  header = (
      f"{'Module (basename)':30} {'Struct':25} {'Fields':>6} "
      f"{'TP':>4} {'FP':>4} {'FN':>4} {'TN':>4} {'Prec':>7} {'Rec':>7}"
  )
  print(header)
  print("-" * len(header))
  total_tp = total_fp = total_fn = total_tn = 0
  for res in results:
    module_name = Path(res.module_id).name
    prec_text = "nan" if math.isnan(res.precision) else f"{res.precision:7.3f}"
    rec_text = "nan" if math.isnan(res.recall) else f"{res.recall:7.3f}"
    print(
        f"{module_name:30} {res.struct_name:25} {res.field_count:6d} "
        f"{res.true_positive:4d} {res.false_positive:4d} "
        f"{res.false_negative:4d} {res.true_negative:4d} "
        f"{prec_text} {rec_text}"
    )
    total_tp += res.true_positive
    total_fp += res.false_positive
    total_fn += res.false_negative
    total_tn += res.true_negative
  agg_precision = float("nan") if (total_tp + total_fp) == 0 else total_tp / (total_tp + total_fp)
  agg_recall = float("nan") if (total_tp + total_fn) == 0 else total_tp / (total_tp + total_fn)
  print("\nOverall confusion matrix:")
  print(f"  TP={total_tp} FP={total_fp} FN={total_fn} TN={total_tn}")
  if math.isfinite(agg_precision):
    print(f"  Precision: {agg_precision:.3f}")
  else:
    print("  Precision: nan (no static-cold predictions)")
  if math.isfinite(agg_recall):
    print(f"  Recall: {agg_recall:.3f}")
  else:
    print("  Recall: nan (no dynamically cold fields)")


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
  results = compute_results(static_data, dynamic_data, args.threshold, args.module, args.struct)
  print_results(results)
  if args.output:
    write_csv(results, args.output)


if __name__ == "__main__":
  main()
