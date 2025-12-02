# Smarter Pool Allocation

This repo has two main parts:
- `candidate-analysis/` – LLVM pass and tooling to produce static/dynamic field hotness reports.
- `cdol-pool-alloc-testing/` – pool allocator passes/runtime + workflow to apply pool allocation.

Use the READMEs in each subdirectory for the authoritative instructions. This top-level file keeps a quick map and summary commands.

## Where to look
- Candidate analysis: build/run, Olden scripts, evaluation → `candidate-analysis/README.md`
- Pool allocator: architecture, CMake build, workflow target details → `cdol-pool-alloc-testing/README.md` and `cdol-pool-alloc-testing/pool-alloc/README.md`

## Quick summary (pool-alloc build)
The submodule respects `LLVM_DIR`/`Clang_DIR` (no hardcoded paths) and enables C and CXX. Build it with the same LLVM as `candidate-analysis` (Homebrew `llvm@19`):
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
Test run of the allocator workflow (from the submodule build dir):
```bash
cd cdol-pool-alloc-testing/pool-alloc/build
PATH="${LLVM_PREFIX}/bin:$PATH" INPUT_FILE=../alloc-std.cpp make workflow
```
Outputs stay in the submodule build dir; avoid `git add cdol-pool-alloc-testing` unless you intend to move the submodule pointer. See `cdol-pool-alloc-testing/README.md` for the full, up-to-date workflow command (including `POOL_ALLOC_TYPES_STRING`).

## Using candidate-analysis to drive pool allocation
1) Run candidate-analysis to produce a `profitability.yaml` (see `candidate-analysis/README.md`).
2) Generate `pool_alloc_types` from that YAML:
```bash
python candidate-analysis/tools/profitability_to_pool_list.py \
  /path/to/profitability.yaml \
  --output /tmp/pool_types.txt
```
3) Run the pool workflow using that list (injects `const char* pool_alloc_types = "<list>";`):
```bash
LLVM_PREFIX="$(brew --prefix llvm@19)"
cd cdol-pool-alloc-testing/pool-alloc/build
PATH="${LLVM_PREFIX}/bin:$PATH" \
  POOL_ALLOC_TYPES_STRING="$(cat /tmp/pool_types.txt)" \
  INPUT_FILE=../alloc-std.cpp \
  make workflow
```
Outputs stay in the submodule build dir. See the pool allocator README for current details.
