#!/usr/bin/env bash

set -euo pipefail

# Build all Olden benchmarks into .linked.bc files for candidate-analysis.
# This mirrors the per-benchmark Makefile flags (mostly -DTORONTO, with a few
# extras) and writes output under $OLDEN_BUILD, defaulting to the canonical
# llvm-test-suite layout.

DEFAULT_LLVM_PREFIX="$(brew --prefix llvm@19 2>/dev/null || true)"
OLDEN_SRC=${OLDEN_SRC:-/Users/haozhi5/Projects/llvm-test-suite/MultiSource/Benchmarks/Olden}
OLDEN_BUILD=${OLDEN_BUILD:-/Users/haozhi5/Projects/llvm-test-suite/build-olden/MultiSource/Benchmarks/Olden}

CLANG_BIN=${CLANG_BIN:-${DEFAULT_LLVM_PREFIX:+${DEFAULT_LLVM_PREFIX}/bin/clang}}
LLVM_LINK_BIN=${LLVM_LINK_BIN:-${DEFAULT_LLVM_PREFIX:+${DEFAULT_LLVM_PREFIX}/bin/llvm-link}}

# Fall back to PATH if brew lookup failed or the tools are not executable.
CLANG_BIN=${CLANG_BIN:-clang}
LLVM_LINK_BIN=${LLVM_LINK_BIN:-llvm-link}

if [[ ! -x "${CLANG_BIN}" ]]; then
  echo "error: clang not found (set CLANG_BIN or LLVM_PREFIX)" >&2
  exit 1
fi
if [[ ! -x "${LLVM_LINK_BIN}" ]]; then
  echo "error: llvm-link not found (set LLVM_LINK_BIN or LLVM_PREFIX)" >&2
  exit 1
fi

# Older macOS Bash lacks associative arrays; use helpers instead.
prog_sources() {
  case "$1" in
    bh) echo "args.c newbh.c walksub.c util.c" ;;
    bisort) echo "args.c bitonic.c" ;;
    em3d) echo "args.c util.c em3d.c make_graph.c main.c" ;;
    health) echo "args.c list.c health.c poisson.c" ;;
    mst) echo "args.c hash.c main.c makegraph.c" ;;
    perimeter) echo "args.c main.c maketree.c" ;;
    power) echo "main.c build.c compute.c" ;;
    treeadd) echo "args.c par-alloc.c node.c" ;;
    tsp) echo "args.c build.c main.c tsp.c" ;;
    voronoi) echo "args.c vector.c newvor.c output.c" ;;
    *) echo "" ;;
  esac
}

prog_extra_cflags() {
  case "$1" in
    bh) echo "-fcommon -Wno-implicit-int" ;;
    *) echo "" ;;
  esac
}

build_prog() {
  local prog="$1"
  local srcs="$2"
  local extra_cflags="${3:-}"

  local src_dir="${OLDEN_SRC}/${prog}"
  local out_dir="${OLDEN_BUILD}/${prog}"

  if [[ ! -d "${src_dir}" ]]; then
    echo "skip ${prog}: source dir missing at ${src_dir}" >&2
    return
  fi

  mkdir -p "${out_dir}"
  rm -f "${out_dir}"/*.bc

  local cflags=(-O0 -emit-llvm -c -DTORONTO ${extra_cflags})

  echo "==> ${prog}: compiling ${srcs}"
  for src in ${srcs}; do
    local src_path="${src_dir}/${src}"
    local bc_out="${out_dir}/${src%.c}.bc"
    if [[ ! -f "${src_path}" ]]; then
      echo "error: ${src_path} not found" >&2
      exit 1
    fi
    "${CLANG_BIN}" "${cflags[@]}" "${src_path}" -o "${bc_out}"
  done

  echo "==> ${prog}: linking -> ${out_dir}/${prog}.linked.bc"
  "${LLVM_LINK_BIN}" "${out_dir}"/*.bc -o "${out_dir}/${prog}.linked.bc"
}

PROGS="bh bisort em3d health mst perimeter power treeadd tsp voronoi"

for prog in ${PROGS}; do
  build_prog "${prog}" "$(prog_sources "${prog}")" "$(prog_extra_cflags "${prog}")"
done

echo
echo "All Olden .linked.bc files are under ${OLDEN_BUILD}"
