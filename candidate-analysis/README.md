# candidate-analysis

Out-of-tree LLVM analysis pass that implements the affinity-driven field classification groundwork for the smarter pool allocation workflow. The project builds as a standalone CMake target and produces a pass plugin that can be loaded with `opt`.

## Building

```
cmake -S . -B build-candidate \  -DLLVM_DIR="$(llvm-config-19 --cmakedir)" \  -DCMAKE_BUILD_TYPE=Release

cmake --build build-candidate --config Release
```

This generates `libCandidateAnalysis.so` inside `build-candidate/lib/`.

## Running

Use `clang` to convert c code to llmv IR code

```
clang -S -emit-llvm -O0 test/inputs/mixed_nested.c -o test/inputs/mixed_nested.ll

```

Use `opt` (from the same LLVM build) to load the plugin and run the pass:

```
"$(llvm-config-19 --bindir)"/opt \
  -load-pass-plugin /home/really146/projects/smarter-pool-allocation/candidate-analysis/build-candidate/lib/libCandidateAnalysis.so \
  -passes=candidate-analysis \
  -disable-output \
  /home/really146/projects/smarter-pool-allocation/candidate-analysis/test/inputs/simple_nested.ll
```

Test scaffolding and dedicated drivers live in the `tools/` and `test/` directories.

## Algorithm Walkthrough
the project's main effort will be spent on an algorithm that focuses on loop analysis, do not do anything yet but understand the algorithm:
The pass builds a loop structure graph, walks each loop’s basic blocks, and collects the struct fields referenced in that loop into a weighted affinity group; non-loop code forms a single group weighted by the routine entry. Identical groups are merged by summing weights. From all groups, we construct a per-type affinity graph with fields as nodes and an edge when two fields co-occur in at least one group; the edge weight is the sum of the corresponding group weights. A field’s hotness is the sum of the incoming edge weights at its node. For weights, we use the paper’s static loop analysis: estimate loop/header edge frequencies and use those as group weights using LLVM loop analysis tools and make sure hot fields in callees reached from hot loops are ranked appropriately. We then apply the paper’s static threshold to classify fields (hot vs. cold) using the same published settings (split threshold is set to 7.5% under static loop analysis)

## Implementation

### FieldRef.h
Handles both GEPs and constant-expression GEPs via GEPOperator. It’s opaque-pointer friendly by using `getSourceElementType()`, It recognizes only direct struct field accesses (one struct index). If you need nested paths (e.g., struct { Inner i; } then field of Inner), you’d iterate all indices and build a path (e.g., [outerField, innerField]). 

If you have array indices before the struct index, you’d need to walk the full index sequence and the corresponding element types at each step.



## Potential Problems to Consider
1. The `FieldRef.h` only handles direct field access, so in terms of pointer chasing situation, the candidate analysis would not do a good job
