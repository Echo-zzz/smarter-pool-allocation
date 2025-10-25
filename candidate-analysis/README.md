# candidate-analysis

Out-of-tree LLVM analysis pass that implements the affinity-driven field classification groundwork for the smarter pool allocation workflow. The project builds as a standalone CMake target and produces a pass plugin that can be loaded with `opt`.

## Building

```
cmake -S . -B build-canaidate \  -DLLVM_DIR="$(llvm-config-19 --cmakedir)" \  -DCMAKE_BUILD_TYPE=Release

cmake --build build-canaidate --config Release
```

This generates `libCandidateAnalysis.so` inside `build-candidate/lib/`.

## Running

Use `opt` (from the same LLVM build) to load the plugin and run the pass:

```
"$(llvm-config-19 --bindir)"/opt \
  -load-pass-plugin /home/really146/projects/smarter-pool-allocation/candidate-analysis/build-canaidate/lib/libCandidateAnalysis.so \
  -passes=candidate-analysis \
  -disable-output \
  /home/really146/projects/smarter-pool-allocation/candidate-analysis/test/inputs/simple_nested.ll
```

Test scaffolding and dedicated drivers live in the `tools/` and `test/` directories.

## Algorithm Walkthrough
the project's main effort will be spent on an algorithm that focuses on loop analysis, do not do anything yet but understand the algorithm:
The pass builds a loop structure graph, walks each loop’s basic blocks, and collects the struct fields referenced in that loop into a weighted affinity group; non-loop code forms a single group weighted by the routine entry. Identical groups are merged by summing weights. From all groups, we construct a per-type affinity graph with fields as nodes and an edge when two fields co-occur in at least one group; the edge weight is the sum of the corresponding group weights. A field’s hotness is the sum of the incoming edge weights at its node. For weights, we use the paper’s static loop analysis: estimate loop/header edge frequencies and use those as group weights using LLVM loop analysis tools and make sure hot fields in callees reached from hot loops are ranked appropriately. We then apply the paper’s static threshold to classify fields (hot vs. cold) using the same published settings (split threshold is set to 7.5% under static loop analysis)

