# candidate-analysis

Out-of-tree LLVM analysis pass that implements the affinity-driven field classification groundwork for the smarter pool allocation workflow. The project builds as a standalone CMake target and produces a pass plugin that can be loaded with `opt`.

## Building

### Linux

```
cmake -S . -B build-candidate \
  -DLLVM_DIR="$(llvm-config-19 --cmakedir)" \
  -DCMAKE_BUILD_TYPE=Release

cmake --build build-candidate --config Release
```

### macOS (Homebrew LLVM 19)

```
LLVM_PREFIX="$(brew --prefix llvm@19)"

cmake -S ./candidate-analysis -B candidate-analysis/build-candidate \
  -DLLVM_DIR="${LLVM_PREFIX}/lib/cmake/llvm" \
  -DCMAKE_BUILD_TYPE=Release

cmake --build candidate-analysis/build-candidate --config Release
```

This generates `libCandidateAnalysis.so` (or `.dylib` on macOS) inside `build-candidate/lib/`.

## Running

The project now ships two passes inside the same plugin:

- `candidate-analysis` (static loop/affinity analysis)
- `field-access-profiler` (instrumentation that records dynamic field counts)

Both rely on LLVM IR input, so start by producing bitcode:

```
clang -S -emit-llvm -O0 test/inputs/mixed_nested.c -o test/inputs/mixed_nested.ll
```

### Static candidate-analysis pass

Run the original analysis to generate the YAML reports:

Linux:

```
"$(llvm-config-19 --bindir)"/opt \
  -load-pass-plugin "$(pwd)"/candidate-analysis/build-candidate/lib/libCandidateAnalysis.so \
  -passes=candidate-analysis \
  -disable-output \
  "$(pwd)"/test/inputs/simple_nested.ll
```

macOS:

```
LLVM_PREFIX="$(brew --prefix llvm@19)"

"${LLVM_PREFIX}/bin/opt" \
  -load-pass-plugin "$(pwd)"/candidate-analysis/build-candidate/lib/libCandidateAnalysis.so \
  -passes=candidate-analysis \
  -disable-output \
  "$(pwd)"/candidate-analysis/test/inputs/simple_nested.ll
```

Reports appear under `candidate-analysis-report/<module-name>/` (type-affinity
and profitability YAML). Override the destination with
`-candidate-analysis-output-dir=/path/to/reports` if desired.

### Dynamic Field Access Profiler

The profiler pass reuses the same field matching logic but instruments the IR
with counters. After instrumentation you must finish compiling and *run* the
program to collect counts.

1. Instrument the module:

   ```
   opt \
     -load-pass-plugin "$(pwd)"/candidate-analysis/build-candidate/lib/libCandidateAnalysis.so \
     -passes=field-access-profiler \
     -o instrumented.bc \
     input.ll
   ```

2. Continue the normal toolchain steps. For example:

   ```
   clang instrumented.bc -o instrumented-bin
   ./instrumented-bin            # running the binary writes the YAML
   ```

   The emitted file is `candidate-analysis-report/<module-name>/field-access-profiler.yaml`.

The dynamic YAML uses the exact same struct labels and field indices as the
static pass, making it easy to compare predictions vs. observed behavior. Use
`-candidate-analysis-output-dir=…` to direct both passes to a different output
root. Test scaffolding and dedicated drivers live in `tools/` and `test/`.

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
