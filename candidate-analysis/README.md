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
