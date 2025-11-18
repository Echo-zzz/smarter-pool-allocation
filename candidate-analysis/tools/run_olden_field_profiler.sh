#!/usr/bin/env bash

set -euo pipefail

# Location of this repo (script lives under candidate-analysis/tools/)
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

# Path to the LLVM test-suite Olden build tree. Override OLDEN_BUILD if needed.
OLDEN_BUILD=${OLDEN_BUILD:-/Users/haozhi5/Projects/llvm-test-suite/build-olden/MultiSource/Benchmarks/Olden}

# LLVM tools + plugin locations. Override via environment if you use custom builds.
OPT_BIN=${OPT_BIN:-opt}
CLANG_BIN=${CLANG_BIN:-clang}
PASS_PLUGIN=${PASS_PLUGIN:-${REPO_ROOT}/build-candidate/lib/libCandidateAnalysis.so}

# Where the profiler should emit YAML at runtime.
OUTPUT_ROOT=${OUTPUT_ROOT:-${REPO_ROOT}/candidate-analysis-report/olden_dynamic}

if [[ ! -x "${PASS_PLUGIN}" ]]; then
  echo "error: pass plugin not found at ${PASS_PLUGIN}" >&2
  exit 1
fi

if [[ ! -d "${OLDEN_BUILD}" ]]; then
  echo "error: Olden build directory not found at ${OLDEN_BUILD}" >&2
  exit 1
fi

mkdir -p "${OUTPUT_ROOT}"

echo "==> Using pass plugin: ${PASS_PLUGIN}"
echo "==> Emitting runtime YAML under: ${OUTPUT_ROOT}"

BC_FILES=()
while IFS= read -r -d '' BC; do
  BC_FILES+=("$BC")
done < <(find "${OLDEN_BUILD}" -name '*.linked.bc' -print0 | sort -z)

if [[ ${#BC_FILES[@]} -eq 0 ]]; then
  echo "No *.linked.bc files found under ${OLDEN_BUILD}" >&2
  exit 1
fi

for BC in "${BC_FILES[@]}"; do
  PROG_DIR="$(dirname "${BC}")"
  PROG_NAME="$(basename "${BC}" .linked.bc)"
  INST_BC="${PROG_DIR}/${PROG_NAME}.field-prof.bc"
  EXE="${PROG_DIR}/${PROG_NAME}.field-prof.bin"

  echo "\n==> Instrumenting ${BC}" 
  "${OPT_BIN}" \
    -load-pass-plugin "${PASS_PLUGIN}" \
    -passes=field-access-profiler \
    -candidate-analysis-output-dir="${OUTPUT_ROOT}" \
    -o "${INST_BC}" \
    "${BC}"

  echo "==> Linking ${INST_BC}"
  "${CLANG_BIN}" "${INST_BC}" -o "${EXE}"

  echo "==> Running ${EXE}"
  "${EXE}" || echo "warning: ${EXE} exited with status $?"
done

echo "\nAll programs processed. Dynamic reports live under ${OUTPUT_ROOT}."
