# candidate-analysis

Out-of-tree LLVM analysis pass that implements the affinity-driven field classification groundwork for the smarter pool allocation workflow. The project builds as a standalone CMake target and produces a pass plugin that can be loaded with `opt`.

## Prerequisites

- LLVM 19 (we regularly test with the Homebrew and source builds) providing `clang`, `opt`, `llvm-config`, and the CMake package files.
- CMake ≥ 3.20 and a C++20-capable compiler (Clang or GCC on Linux, Xcode toolchain on macOS).
- Python 3.9+ with `pyyaml` (install via `python3 -m pip install --user pyyaml`) for the evaluation tooling.
- The LLVM test-suite’s Olden benchmark build directories if you want to replay the full benchmark evaluation (see below).

## Building the pass plugin

> Build from the repository root (this folder contains `CMakeLists.txt`).

### Linux

```bash
cmake -S . -B build-candidate \
  -DLLVM_DIR="$(llvm-config-19 --cmakedir)" \
  -DCMAKE_BUILD_TYPE=Release

cmake --build build-candidate --config Release
```

### macOS (Homebrew LLVM 19)

```bash
LLVM_PREFIX="$(brew --prefix llvm@19)"

cmake -S ./candidate-analysis -B candidate-analysis/build-candidate \
  -DLLVM_DIR="${LLVM_PREFIX}/lib/cmake/llvm" \
  -DCMAKE_BUILD_TYPE=Release

cmake --build candidate-analysis/build-candidate --config Release
```

Both commands generate `build-candidate/lib/libCandidateAnalysis.so` (or `.dylib` on macOS). All subsequent commands refer to this shared object as the “pass plugin”.

## Producing bitcode for tests

The tree already contains several toy programs under `test/inputs/`. Produce LLVM IR for any of them before running a pass:

```bash
clang -S -emit-llvm -O0 test/inputs/mixed_nested.c -o test/inputs/mixed_nested.ll
```

Use `-candidate-analysis-output-dir=/path/to/reports` to control where pass outputs land; otherwise they go to `candidate-analysis-report/<module-name>/`.

## Running the passes manually

The plugin ships two passes:

- `candidate-analysis` (static loop/affinity analysis → type-affinity/profitability YAML)
- `field-access-profiler` (instruments IR and logs runtime field hotness)

### Static candidate-analysis pass

Linux:

```bash
"$(llvm-config-19 --bindir)"/opt \
  -load-pass-plugin "$(pwd)"/candidate-analysis/build-candidate/lib/libCandidateAnalysis.so \
  -passes=candidate-analysis \
  -disable-output \
  "$(pwd)"/candidate-analysis/test/inputs/simple_nested.ll
```

macOS:

```bash
LLVM_PREFIX="$(brew --prefix llvm@19)"

OUTDIR="/tmp/cand-analysis-out"

"${LLVM_PREFIX}/bin/opt" \
-load-pass-plugin "$(pwd)"/candidate-analysis/build-candidate/lib/libCandidateAnalysis.so \
-passes=candidate-analysis \
-candidate-analysis-output-dir="${OUTDIR}" \
-disable-output \
"$(pwd)"/candidate-analysis/test/inputs/simple.ll
```

The pass emits `type-affinity.yaml` and `profitability.yaml` under the report directory.

### Dynamic field-access profiler

1. Instrument the module:

   ```bash
   opt \
     -load-pass-plugin "$(pwd)"/candidate-analysis/build-candidate/lib/libCandidateAnalysis.so \
     -passes=field-access-profiler \
     -candidate-analysis-output-dir="$(pwd)/candidate-analysis-report/dynamic_test" \
     -o instrumented.bc \
     candidate-analysis/test/inputs/mixed_nested.ll
   ```

2. Build and run the instrumented binary to materialize the counters:

   ```bash
   clang instrumented.bc -o instrumented-bin
   ./instrumented-bin
   ```

Each run produces `field-access-profiler.yaml` for the module under the specified output directory.

## Automating the Olden benchmark experiments

The `tools/` directory contains convenience scripts for static and dynamic sweeps. Here is an end-to-end run starting from a fresh llvm-test-suite checkout.

1) Clone llvm-test-suite and set the LLVM prefix (Homebrew LLVM 19):
```bash
git clone https://github.com/llvm/llvm-test-suite.git /Users/haozhi5/Projects/llvm-test-suite
LLVM_PREFIX="$(brew --prefix llvm@19)"
```

2) Configure + build Olden with embedded bitcode (CMake + Ninja):
```bash
cd /Users/haozhi5/Projects/llvm-test-suite
cmake -S . -B build-olden -G Ninja \
  -DTEST_SUITE_SUBDIRS="MultiSource/Benchmarks/Olden" \
  -DTEST_SUITE_RUN_BENCHMARKS=OFF \
  -DCMAKE_C_COMPILER="${LLVM_PREFIX}/bin/clang" \
  -DCMAKE_CXX_COMPILER="${LLVM_PREFIX}/bin/clang++" \
  -DCMAKE_C_FLAGS="--save-temps=obj -O0 -g -fembed-bitcode -fno-unroll-loops -fno-vectorize -fno-discard-value-names" \
  -DCMAKE_CXX_FLAGS="--save-temps=obj -O0 -g -fembed-bitcode -fno-unroll-loops -fno-vectorize -fno-discard-value-names" \
  -DCMAKE_BUILD_TYPE=Debug
ninja -C build-olden
```

3) Build the candidate-analysis plugin:
```bash
cd /Users/haozhi5/Projects/smarter-pool-allocation
cmake -S ./candidate-analysis -B candidate-analysis/build-candidate \
  -DLLVM_DIR="${LLVM_PREFIX}/lib/cmake/llvm" \
  -DCMAKE_BUILD_TYPE=Release
cmake --build candidate-analysis/build-candidate --config Release
```

4) Generate `.linked.bc` for static analysis (uses clang + llvm-link):
```bash
cd candidate-analysis
LLVM_PREFIX="$(brew --prefix llvm@19)" \
OLDEN_SRC=/Users/haozhi5/Projects/llvm-test-suite/MultiSource/Benchmarks/Olden \
OLDEN_BUILD=/Users/haozhi5/Projects/llvm-test-suite/build-olden/MultiSource/Benchmarks/Olden \
CLANG_BIN="${LLVM_PREFIX}/bin/clang" \
LLVM_LINK_BIN="${LLVM_PREFIX}/bin/llvm-link" \
./tools/build_olden_bitcode.sh
```

5) Static pass over Olden:
```bash
cd candidate-analysis
OLDEN_BUILD=/Users/haozhi5/Projects/llvm-test-suite/build-olden/MultiSource/Benchmarks/Olden \
OPT_BIN="${LLVM_PREFIX}/bin/opt" \
PASS_PLUGIN=$(pwd)/build-candidate/lib/libCandidateAnalysis.so \
OUTPUT_ROOT=$(pwd)/candidate-analysis-report/olden_static \
./tools/run_olden_static_analysis.sh
```

6) Dynamic field-access profiler over Olden (uses TU .bc from the CMake build):
```bash
cd candidate-analysis
OLDEN_BUILD=/Users/haozhi5/Projects/llvm-test-suite/build-olden/MultiSource/Benchmarks/Olden \
OPT_BIN="${LLVM_PREFIX}/bin/opt" \
CLANG_BIN="${LLVM_PREFIX}/bin/clang" \
LLVM_LINK_BIN="${LLVM_PREFIX}/bin/llvm-link" \
PASS_PLUGIN=$(pwd)/build-candidate/lib/libCandidateAnalysis.so \
OUTPUT_ROOT=$(pwd)/candidate-analysis-report/olden_dynamic \
./tools/run_olden_dynamic_from_build.sh
```

- Static reports land under `candidate-analysis-report/olden_static`.
- Dynamic reports land under `candidate-analysis-report/olden_dynamic`.
- If you prefer not to embed bitcode via CMake, you can still produce `.linked.bc` with `build_olden_bitcode.sh` and use `run_olden_field_profiler.sh` (expects existing `.linked.bc` files).

### Static pass over Olden

```bash
cd candidate-analysis
OLDEN_BUILD=/path/to/llvm-test-suite/build-olden/MultiSource/Benchmarks/Olden \
OPT_BIN=/path/to/opt \
PASS_PLUGIN=$(pwd)/build-candidate/lib/libCandidateAnalysis.so \
OUTPUT_ROOT=$(pwd)/candidate-analysis-report/olden_static \
./tools/run_olden_static_analysis.sh
```

Each `.linked.bc` is processed once; the reports end up under `$OUTPUT_ROOT`.

### Dynamic profiler over Olden

```bash
cd candidate-analysis
OLDEN_BUILD=/path/to/llvm-test-suite/build-olden/MultiSource/Benchmarks/Olden \
OPT_BIN=/path/to/opt \
CLANG_BIN=/path/to/clang \
PASS_PLUGIN=$(pwd)/build-candidate/lib/libCandidateAnalysis.so \
OUTPUT_ROOT=$(pwd)/candidate-analysis-report/olden_dynamic \
./tools/run_olden_field_profiler.sh
```

The script instruments every benchmark, links the executable, and runs it to emit runtime counts. Expect binaries (`*.field-prof.bin`) alongside the `.linked.bc` sources; their YAML lives under `$OUTPUT_ROOT`.

If you build Olden via CMake with `-fembed-bitcode --save-temps=obj` (for example, using Ninja and `CMAKE_C_FLAGS="--save-temps=obj -O0 -g -fembed-bitcode -fno-unroll-loops -fno-vectorize -fno-discard-value-names"`), you can run the dynamic sweep directly from the build tree:

```bash
cd candidate-analysis
LLVM_PREFIX="$(brew --prefix llvm@19)"
OLDEN_BUILD=/Users/haozhi5/Projects/llvm-test-suite/build-olden/MultiSource/Benchmarks/Olden \
OPT_BIN="${LLVM_PREFIX}/bin/opt" \
CLANG_BIN="${LLVM_PREFIX}/bin/clang" \
LLVM_LINK_BIN="${LLVM_PREFIX}/bin/llvm-link" \
PASS_PLUGIN=$(pwd)/build-candidate/lib/libCandidateAnalysis.so \
OUTPUT_ROOT=$(pwd)/candidate-analysis-report/olden_dynamic \
./tools/run_olden_dynamic_from_build.sh
```

The script will:
- find TU `.bc` files under each `CMakeFiles/<prog>.dir`,
- `llvm-link` them into `<prog>.linked.bc`,
- instrument with `field-access-profiler`,
- link and run the executable to materialize `field-access-profiler.yaml` under `$OUTPUT_ROOT`.

## Evaluating static vs. dynamic rankings

After collecting both static and dynamic reports, compare their field hotness rankings using the Spearman rho utility:

```bash
cd candidate-analysis
python3 tools/eval_rank_correlation.py \
  --static-dir candidate-analysis-report/olden_static \
  --dynamic-dir candidate-analysis-report/olden_dynamic \
  --output spearman.csv
```

Install PyYAML first (`python3 -m pip install --user pyyaml`). Optional flags `--module` and `--struct` let you focus on specific reports, and `--output` writes a CSV for downstream plotting. The script prints one line per struct plus an aggregate mean rho.

## Evaluating cold precision

Given static reports (e.g., `candidate-analysis-report/olden_static`) and dynamic reports (e.g., `candidate-analysis-report/olden_dynamic`), compute cold precision with:

```bash
cd candidate-analysis
python3 tools/eval_cold_precision.py \
  --static-dir candidate-analysis-report/olden_static \
  --dynamic-dir candidate-analysis-report/olden_dynamic \
  --output cold_precision.csv
```

Install PyYAML first (`python3 -m pip install --user pyyaml`). The script emits per-struct cold precision and writes a CSV when `--output` is provided.

## Evaluating static vs. dynamic rankings

After collecting both static and dynamic reports, compare their field hotness rankings using the Spearman rho utility:

```bash
cd candidate-analysis
python3 tools/eval_rank_correlation.py \
  --static-dir candidate-analysis-report/olden_static \
  --dynamic-dir candidate-analysis-report/olden_dynamic \
  --output spearman.csv
```

Install PyYAML first (`python3 -m pip install --user pyyaml`). Optional flags `--module` and `--struct` let you focus on specific reports, and `--output` writes a CSV for downstream plotting. The script prints one line per struct plus an aggregate mean rho.

## Algorithm Walkthrough
the project's main effort will be spent on an algorithm that focuses on loop analysis, the algorithm is as follows:
The pass builds a loop structure graph, walks each loop’s basic blocks, and collects the struct fields referenced in that loop into a weighted affinity group; non-loop code forms a single group weighted by the routine entry. Identical groups are merged by summing weights. From all groups, we construct a per-type affinity graph with fields as nodes and an edge when two fields co-occur in at least one group; the edge weight is the sum of the corresponding group weights. A field’s hotness is the sum of the incoming edge weights at its node. For weights, we use the paper’s static loop analysis: estimate loop/header edge frequencies and use those as group weights using LLVM loop analysis tools and make sure hot fields in callees reached from hot loops are ranked appropriately. We then apply the paper’s static threshold to classify fields (hot vs. cold) using the same published settings (split threshold is set to 7.5% under static loop analysis)

## Algorithm Walkthrough
the project's main effort will be spent on an algorithm that focuses on loop analysis, do not do anything yet but understand the algorithm:
The pass builds a loop structure graph, walks each loop’s basic blocks, and collects the struct fields referenced in that loop into a weighted affinity group; non-loop code forms a single group weighted by the routine entry. Identical groups are merged by summing weights. From all groups, we construct a per-type affinity graph with fields as nodes and an edge when two fields co-occur in at least one group; the edge weight is the sum of the corresponding group weights. A field’s hotness is the sum of the incoming edge weights at its node. For weights, we use the paper’s static loop analysis: estimate loop/header edge frequencies and use those as group weights using LLVM loop analysis tools and make sure hot fields in callees reached from hot loops are ranked appropriately. We then apply the paper’s static threshold to classify fields (hot vs. cold) using the same published settings (split threshold is set to 7.5% under static loop analysis)

## Implementation

### FieldRef.h
Handles both GEPs and constant-expression GEPs via GEPOperator. It’s opaque-pointer friendly by using `getSourceElementType()`, It recognizes only direct struct field accesses (one struct index). If you need nested paths (e.g., struct { Inner i; } then field of Inner), you’d iterate all indices and build a path (e.g., [outerField, innerField]). 

If you have array indices before the struct index, you’d need to walk the full index sequence and the corresponding element types at each step.



## Potential Problems to Consider
1. The `FieldRef.h` only handles direct field access, so in terms of pointer chasing situation, the candidate analysis would not do a good job
2. The aggregation of the weights only work within a single module, need to decide if intro-procedure consideration is needed
3. Should we provide edge weights (affinity between fields) in the pass? It is not required for the candidate selection, but might be very helpful for future data splitting as they provide information on how often fields are accessed togethers
