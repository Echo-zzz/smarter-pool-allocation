#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

OLDEN_BUILD=${OLDEN_BUILD:-/Users/haozhi5/Projects/llvm-test-suite/build-olden/MultiSource/Benchmarks/Olden}
OPT_BIN=${OPT_BIN:-opt}
PASS_PLUGIN=${PASS_PLUGIN:-${REPO_ROOT}/build-candidate/lib/libCandidateAnalysis.so}
OUTPUT_ROOT=${OUTPUT_ROOT:-${REPO_ROOT}/candidate-analysis-report/olden_static}

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
echo "==> Writing static reports under: ${OUTPUT_ROOT}"

BC_FILES=()
while IFS= read -r -d '' BC; do
  BC_FILES+=("$BC")
done < <(find "${OLDEN_BUILD}" -name '*.linked.bc' -print0 | sort -z)

if [[ ${#BC_FILES[@]} -eq 0 ]]; then
  echo "No *.linked.bc files found under ${OLDEN_BUILD}" >&2
  exit 1
fi

for BC in "${BC_FILES[@]}"; do
  echo
  echo "==> Running candidate-analysis on ${BC}"
  "${OPT_BIN}" \
    -load-pass-plugin "${PASS_PLUGIN}" \
    -passes=candidate-analysis \
    -candidate-analysis-output-dir="${OUTPUT_ROOT}" \
    -disable-output \
    "${BC}"
done

echo
echo "All programs processed. Static reports live under ${OUTPUT_ROOT}."

