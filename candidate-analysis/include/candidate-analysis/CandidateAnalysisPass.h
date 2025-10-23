#pragma once

#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/ADT/SmallVector.h"
#include <vector>

namespace llvm
{
  class Module;
  struct PassPluginLibraryInfo;
  class Loop;
  class Function;
} // namespace llvm

namespace candidate
{

  /// Module-level analysis pass that will compute struct-field affinity data.
  class CandidateAnalysisPass : public llvm::PassInfoMixin<CandidateAnalysisPass>
  {
  public:
    llvm::PreservedAnalyses run(llvm::Module &M, llvm::ModuleAnalysisManager &MAM);

  private:
    // Compact representation for a loop node in the per-function loop tree.
    struct LoopNode
    {
      const llvm::Loop *LoopRef = nullptr;
      int Parent = -1;
      llvm::SmallVector<int, 4> Children;
    };

    // Aggregated loop tree container for a single function.
    struct FunctionLoopGraph
    {
      std::vector<LoopNode> Nodes;
    };

    static void dumpLoopGraph(const llvm::Function &F, const FunctionLoopGraph &LoopGraph);
  };

  /// Registration helper exposed so unit tests or custom drivers can attach the
  /// pass to an arbitrary pipeline without relying on implicit static
  /// initialisers.
  llvm::PassPluginLibraryInfo getCandidateAnalysisPluginInfo();

} // namespace candidate
