#!/usr/bin/env bash

set -euo pipefail

# Instruments and runs the Olden benchmarks using the field-access-profiler
# pass. Assumes you already built the llvm-test-suite Olden targets with
# -fembed-bitcode/--save-temps so TU .bc files exist under CMakeFiles/*.dir.

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

DEFAULT_LLVM_PREFIX="$(brew --prefix llvm@19 2>/dev/null || true)"

OLDEN_BUILD=${OLDEN_BUILD:-/Users/haozhi5/Projects/llvm-test-suite/build-olden/MultiSource/Benchmarks/Olden}
OUTPUT_ROOT=${OUTPUT_ROOT:-${REPO_ROOT}/candidate-analysis-report/olden_dynamic}
PASS_PLUGIN=${PASS_PLUGIN:-${REPO_ROOT}/build-candidate/lib/libCandidateAnalysis.so}

OPT_BIN=${OPT_BIN:-${DEFAULT_LLVM_PREFIX:+${DEFAULT_LLVM_PREFIX}/bin/opt}}
CLANG_BIN=${CLANG_BIN:-${DEFAULT_LLVM_PREFIX:+${DEFAULT_LLVM_PREFIX}/bin/clang}}
LLVM_LINK_BIN=${LLVM_LINK_BIN:-${DEFAULT_LLVM_PREFIX:+${DEFAULT_LLVM_PREFIX}/bin/llvm-link}}

OPT_BIN=${OPT_BIN:-opt}
CLANG_BIN=${CLANG_BIN:-clang}
LLVM_LINK_BIN=${LLVM_LINK_BIN:-llvm-link}

if [[ ! -x "${PASS_PLUGIN}" ]]; then
  echo "error: pass plugin not found at ${PASS_PLUGIN}" >&2
  exit 1
fi
if [[ ! -x "${OPT_BIN}" ]]; then
  echo "error: opt not found (set OPT_BIN or LLVM_PREFIX)" >&2
  exit 1
fi
if [[ ! -x "${CLANG_BIN}" ]]; then
  echo "error: clang not found (set CLANG_BIN or LLVM_PREFIX)" >&2
  exit 1
fi
if [[ ! -x "${LLVM_LINK_BIN}" ]]; then
  echo "error: llvm-link not found (set LLVM_LINK_BIN or LLVM_PREFIX)" >&2
  exit 1
fi
if [[ ! -d "${OLDEN_BUILD}" ]]; then
  echo "error: Olden build directory not found at ${OLDEN_BUILD}" >&2
  exit 1
fi

mkdir -p "${OUTPUT_ROOT}"

echo "==> Using pass plugin: ${PASS_PLUGIN}"
echo "==> Writing dynamic reports under: ${OUTPUT_ROOT}"
echo

build_and_run_prog() {
  local prog_dir="$1"
  local prog="$(basename "${prog_dir}")"
  local linked_bc="${prog_dir}/${prog}.linked.bc"
  local inst_bc="${prog_dir}/${prog}.field-prof.bc"
  local exe="${prog_dir}/${prog}.field-prof.bin"

  # Drop stale artifacts to avoid accidental re-link of already-linked modules.
  rm -f "${linked_bc}" "${inst_bc}" "${exe}"

  local bc_files=()
  local seen=""

  # Prefer TU bitcode under CMakeFiles/ (produced by save-temps); fall back to
  # any top-level .bc only if we haven't already picked that TU.
  while IFS= read -r bc; do
    local base
    base="$(basename "${bc}")"
    if [[ "${seen}" == *"|${base}|"* ]]; then
      continue
    fi
    bc_files+=("${bc}")
    seen+="|${base}|"
  done < <(find "${prog_dir}" -type f -path '*/CMakeFiles/*' -name '*.bc' \
             ! -name 'cmTC_*.bc' \
             ! -name '*.linked.bc' \
             ! -name '*.field-prof.bc' \
             -print)

  while IFS= read -r bc; do
    local base
    base="$(basename "${bc}")"
    if [[ "${seen}" == *"|${base}|"* ]]; then
      continue
    fi
    bc_files+=("${bc}")
    seen+="|${base}|"
  done < <(find "${prog_dir}" -type f ! -path '*/CMakeFiles/*' -name '*.bc' \
             ! -name 'cmTC_*.bc' \
             ! -name '*.linked.bc' \
             ! -name '*.field-prof.bc' \
             -print)

  if [[ ${#bc_files[@]} -eq 0 ]]; then
    echo "skip ${prog}: no .bc files under ${prog_dir}"
    return
  fi

  echo "==> ${prog}: linking ${#bc_files[@]} TU .bc files -> ${linked_bc}"
  "${LLVM_LINK_BIN}" "${bc_files[@]}" -o "${linked_bc}"

  echo "==> ${prog}: instrumenting -> ${inst_bc}"
  "${OPT_BIN}" \
    -load-pass-plugin "${PASS_PLUGIN}" \
    -passes=field-access-profiler \
    -candidate-analysis-output-dir="${OUTPUT_ROOT}" \
    -o "${inst_bc}" \
    "${linked_bc}"

  echo "==> ${prog}: linking executable -> ${exe}"
  "${CLANG_BIN}" "${inst_bc}" -o "${exe}"

  echo "==> ${prog}: running ${exe}"
  if ! "${exe}"; then
    echo "warning: ${exe} exited with non-zero status" >&2
  fi
}

while IFS= read -r -d '' prog_dir; do
  build_and_run_prog "${prog_dir}"
  echo
done < <(find "${OLDEN_BUILD}" -maxdepth 1 -mindepth 1 -type d -print0 | sort -z)

echo "All programs processed (where .bc was present). Dynamic reports live under ${OUTPUT_ROOT}."
