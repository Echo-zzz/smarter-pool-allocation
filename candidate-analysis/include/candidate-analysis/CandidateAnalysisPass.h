#pragma once

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include <vector>
#include <system_error>

namespace llvm
{
  class Module;
  struct PassPluginLibraryInfo;
  class Loop;
  class LoopInfo;
  class Function;
  class DominatorTree;
  class BlockFrequencyInfo;
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
    struct FieldIDInfo
    {
      static FieldID getEmptyKey()
      {
        return {reinterpret_cast<llvm::StructType *>(-1), ~0u};
      }

      static FieldID getTombstoneKey()
      {
        return {reinterpret_cast<llvm::StructType *>(-2), ~0u};
      }

      static unsigned getHashValue(const FieldID &F)
      {
        return static_cast<unsigned>((reinterpret_cast<uintptr_t>(F.ST) >> 3) ^ (F.FieldIndex * 1315423911u));
      }

      static bool isEqual(const FieldID &LHS, const FieldID &RHS)
      {
        return LHS == RHS;
      }
    };

    // A loop- (or non-loop-) scoped group of fields with a weight.
    struct AffinityGroup
    {
      const llvm::Function *F = nullptr;    // provenance (debug)
      int LoopNodeIndex = -1;               // -1 for “non-loop” group
      llvm::DenseMap<FieldID, unsigned, FieldIDInfo> Fields; // frequency of each field use
      double Weight = 0.0;                  // to be filled later
    };

    struct TypeAffinityGraph
    {
      // Aggregate “hotness” for each field index of the struct.
      llvm::DenseMap<unsigned, double> FieldHotness;
      // Undirected edge weights between field indices, stored with canonicalised endpoints.
      llvm::DenseMap<unsigned, llvm::DenseMap<unsigned, double>> EdgeWeights;
    };

    struct FieldScore
    {
      double Hotness = 0.0;           // Absolute hotness from BlockFrequencyInfo
      double RelativeHotness = 0.0;   // Percentage relative to the hottest field
      bool IsCold = false;            // Relative hotness falls below the cold threshold
    };

    struct StructProfitability
    {
      llvm::DenseMap<unsigned, FieldScore> FieldScores;
      double MaxHotness = 0.0;
      bool IsCandidate = false;
    };

    // Collect one AffinityGroup per loop (and optionally one non-loop group).
    // If BFI is provided, we'll fill Weight from the loop header block frequency.
    static std::vector<AffinityGroup>
    collectLoopFieldRefs(const llvm::Function &F,
                         const FunctionLoopGraph &LoopGraph,
                         const llvm::LoopInfo &LI,
                         const llvm::DominatorTree *DT,
                         const llvm::BlockFrequencyInfo *BFI = nullptr);

    static void dumpLoopGraph(const llvm::Function &F, const FunctionLoopGraph &LoopGraph);
    static void dumpLoopFieldRefs(const llvm::Function &F, const FunctionLoopGraph &LoopGraph,
                                  const llvm::LoopInfo &LI, const llvm::DominatorTree *DT);

    static std::vector<AffinityGroup>
    mergeIdenticalGroupsByKey(const std::vector<AffinityGroup> &Groups);

    static void dumpAffinityGroups(const std::vector<AffinityGroup> &Groups);
    static void dumpMergedGroups(const std::vector<AffinityGroup> &MergedGroups);

    static llvm::DenseMap<llvm::StructType *, TypeAffinityGraph>
    buildTypeAffinityGraphs(const std::vector<AffinityGroup> &MergedGroups);
    static llvm::DenseMap<llvm::StructType *, StructProfitability>
    analyzeStructProfitability(const llvm::DenseMap<llvm::StructType *, TypeAffinityGraph> &Graphs);

    static void writeYamlReports(const llvm::Module &M,
                                 const llvm::DenseMap<llvm::StructType *, TypeAffinityGraph> &Graphs,
                                 const llvm::DenseMap<llvm::StructType *, StructProfitability> &Profit);
    static std::error_code
    writeTypeAffinityYaml(llvm::StringRef FilePath, llvm::StringRef ModuleId,
                          const llvm::DenseMap<llvm::StructType *, TypeAffinityGraph> &Graphs);
    static std::error_code
    writeProfitabilityYaml(llvm::StringRef FilePath, llvm::StringRef ModuleId,
                           const llvm::DenseMap<llvm::StructType *, StructProfitability> &Profit);
  };

  /// Registration helper exposed so unit tests or custom drivers can attach the
  /// pass to an arbitrary pipeline without relying on implicit static
  /// initialisers.
  llvm::PassPluginLibraryInfo getCandidateAnalysisPluginInfo();

} // namespace candidate
