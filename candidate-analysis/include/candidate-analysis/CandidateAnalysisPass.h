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

    struct FieldID
    {
      llvm::StructType *ST = nullptr;
      unsigned FieldIndex = ~0u;

      bool operator==(const FieldID &O) const
      {
        return ST == O.ST && FieldIndex == O.FieldIndex;
      }
    };
    struct FieldIDHash
    {
      size_t operator()(const FieldID &F) const
      {
        return std::hash<void *>()(F.ST) ^ (static_cast<size_t>(F.FieldIndex) * 1315423911u);
      }
    };

    // A loop- (or non-loop-) scoped group of fields with a weight.
    struct AffinityGroup
    {
      const llvm::Function *F = nullptr;    // provenance (debug)
      int LoopNodeIndex = -1;               // -1 for “non-loop” group
      llvm::SmallVector<FieldID, 8> Fields; // keep unique fields in this group
      double Weight = 0.0;                  // to be filled later
    };

    // Collect one AffinityGroup per loop (and optionally one non-loop group).
    // If BFI is provided, we'll fill Weight from the loop header block frequency.
    static std::vector<AffinityGroup>
    collectLoopFieldRefs(const llvm::Function &F,
                         const FunctionLoopGraph &LoopGraph,
                         const llvm::BlockFrequencyInfo *BFI = nullptr);

    static void dumpLoopGraph(const llvm::Function &F, const FunctionLoopGraph &LoopGraph);
    static void dumpLoopFieldRefs(const llvm::Function &F, const FunctionLoopGraph &LoopGraph);
  };

  /// Registration helper exposed so unit tests or custom drivers can attach the
  /// pass to an arbitrary pipeline without relying on implicit static
  /// initialisers.
  llvm::PassPluginLibraryInfo getCandidateAnalysisPluginInfo();

} // namespace candidate
