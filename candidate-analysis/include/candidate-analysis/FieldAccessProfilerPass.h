#pragma once

#include "candidate-analysis/FieldID.h"
#include "llvm/IR/PassManager.h"

namespace llvm
{
  class Module;
} // namespace llvm

namespace candidate
{

  /// Module pass that instruments struct-field accesses with counters and emits
  /// a YAML report at runtime.
  class FieldAccessProfilerPass : public llvm::PassInfoMixin<FieldAccessProfilerPass>
  {
  public:
    llvm::PreservedAnalyses run(llvm::Module &M, llvm::ModuleAnalysisManager &MAM);
  };

} // namespace candidate
