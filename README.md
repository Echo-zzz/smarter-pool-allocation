# Smarter Pool Allocation

## cdol-pool-alloc-testing build notes
- The submodule now respects `LLVM_DIR`/`Clang_DIR` (no hardcoded paths) and enables C and CXX so LLVM’s C checks (FFI/libedit) succeed. The Clang plugin links against `clang-cpp` + `LLVMSupport`; the LLVM pass links the needed LLVM components.
- Build the submodule with the same toolchain as `candidate-analysis` (Homebrew `llvm@19`):
  ```bash
  LLVM_PREFIX="$(brew --prefix llvm@19)"
  rm -rf cdol-pool-alloc-testing/pool-alloc/build
  cmake -S cdol-pool-alloc-testing/pool-alloc -B cdol-pool-alloc-testing/pool-alloc/build \
    -DLLVM_DIR="${LLVM_PREFIX}/lib/cmake/llvm" \
    -DClang_DIR="${LLVM_PREFIX}/lib/cmake/clang" \
    -DCMAKE_CXX_COMPILER="${LLVM_PREFIX}/bin/clang++" \
    -DCMAKE_C_COMPILER="${LLVM_PREFIX}/bin/clang"
  cmake --build cdol-pool-alloc-testing/pool-alloc/build
  ```
- Test run of the allocator workflow (from the submodule build dir):
  ```bash
  cd cdol-pool-alloc-testing/pool-alloc/build
  PATH="${LLVM_PREFIX}/bin:$PATH" INPUT_FILE=../alloc-std.cpp make workflow
  ```
  Outputs stay in the submodule build dir; avoid `git add cdol-pool-alloc-testing` in the superproject unless you intend to move the submodule pointer.

## Pool allocator workflow driven by candidate-analysis output (no pass changes)
- Run candidate-analysis to produce a `profitability.yaml` for your module (see `candidate-analysis/README.md` for build/run). Note the path to that YAML under `candidate-analysis-report/.../profitability.yaml`.
- Generate `pool_alloc_types` from that `profitability.yaml`:
  ```bash
  python candidate-analysis/tools/profitability_to_pool_list.py \
    /path/to/profitability.yaml \
    --output /tmp/pool_types.txt
  ```
- Run the pool workflow using that list (injects `const char* pool_alloc_types = "<list>";` before the existing pass pipeline):
  ```bash
  LLVM_PREFIX="$(brew --prefix llvm@19)"
  cd cdol-pool-alloc-testing/pool-alloc/build
  PATH="${LLVM_PREFIX}/bin:$PATH" \
    POOL_ALLOC_TYPES_STRING="$(cat /tmp/pool_types.txt)" \
    INPUT_FILE=../alloc-std.cpp \
    make workflow
  ```
  This keeps the pool pass untouched; it still reads `pool_alloc_types`, but the list is derived from the analysis output. Outputs stay in the submodule build dir; avoid committing the submodule unless you mean to move its pointer.
