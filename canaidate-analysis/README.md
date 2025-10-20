# canaidate-analysis

Out-of-tree LLVM analysis pass that implements the affinity-driven field classification groundwork for the smarter pool allocation workflow. The project builds as a standalone CMake target and produces a pass plugin that can be loaded with `opt`.

## Building

```
cmake -S canaidate-analysis -B build-canaidate \
      -DLLVM_DIR=/path/to/llvm/lib/cmake/llvm
cmake --build build-canaidate
```

This generates `libCanaidateAnalysis.so` inside `build-canaidate/lib/`.

## Running

Use `opt` (from the same LLVM build) to load the plugin and run the pass:

```
opt -load-pass-plugin build-canaidate/lib/libCanaidateAnalysis.so \
    -passes=canaidate-analysis example.ll -disable-output
```

Test scaffolding and dedicated drivers live in the `tools/` and `test/` directories.
