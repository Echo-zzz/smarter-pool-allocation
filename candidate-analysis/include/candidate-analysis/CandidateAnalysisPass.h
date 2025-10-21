#pragma once

#include "llvm/IR/PassManager.h"

namespace llvm {
class Module;
class ModuleAnalysisManager;
struct PassPluginLibraryInfo;
} // namespace llvm

namespace candidate {

/// Module-level analysis pass that will compute struct-field affinity data.
class CandidateAnalysisPass : public llvm::PassInfoMixin<CandidateAnalysisPass> {
public:
  llvm::PreservedAnalyses run(llvm::Module &M, llvm::ModuleAnalysisManager &MAM);
};

/// Registration helper exposed so unit tests or custom drivers can attach the
/// pass to an arbitrary pipeline without relying on implicit static
/// initialisers.
llvm::PassPluginLibraryInfo getCandidateAnalysisPluginInfo();

} // namespace candidate
