#include "candidate-analysis/CandidateAnalysisPass.h"
#include "candidate-analysis/FieldRef.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/BlockFrequencyInfo.h"

#define DEBUG_TYPE "candidate-analysis"

using namespace llvm;

namespace candidate
{

  PreservedAnalyses CandidateAnalysisPass::run(Module &M, ModuleAnalysisManager &MAM)
  {
    // LLVM_DEBUG(dbgs() << "[candidate-analysis] Visiting module: " << M.getName() << '\n');

    // TODO: Implement loop-affinity analysis here.

    // pull the per-function analysis manager from the module pass
    FunctionAnalysisManager &FAM = MAM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
    for (Function &F : M)
    {
      if (F.isDeclaration())
        continue;

      // Ask for loop info for the function
      LoopInfo &LI = FAM.getResult<LoopAnalysis>(F);

      FunctionLoopGraph LoopGraph;
      llvm::DenseMap<const Loop *, int> LoopIndex;

      // Inserts loop into graph, wiring it to parent if already visited.
      auto recordLoop = [&](Loop *L)
      {
        int Index = LoopGraph.Nodes.size();
        LoopGraph.Nodes.emplace_back();
        LoopNode &Node = LoopGraph.Nodes.back();
        Node.LoopRef = L;
        LoopIndex[L] = Index;

        if (Loop *Parent = L->getParentLoop())
        {
          auto It = LoopIndex.find(Parent);
          if (It != LoopIndex.end())
          {
            Node.Parent = It->second;
            LoopGraph.Nodes[Node.Parent].Children.push_back(Index);
          }
        }
      };

      // walk the natural loops
      for (Loop *Root : LI)
      {
        SmallVector<Loop *, 8> stack{Root};
        while (!stack.empty())
        {
          Loop *L = stack.pop_back_val();

          if (!LoopIndex.count(L))
            recordLoop(L);

          for (Loop *SL : L->getSubLoops())
            stack.push_back(SL);
        }
      }

      dumpLoopGraph(F, LoopGraph);
      dumpLoopFieldRefs(F, LoopGraph);

      const BlockFrequencyInfo *BFI = nullptr; // (thread real BFI later)
      auto Groups = collectLoopFieldRefs(F, LoopGraph, BFI);

      // For now, just debug-print group contents to verify:
      for (const auto &G : Groups)
      {
        outs() << "[candidate-analysis] group loop#" << G.LoopNodeIndex << " fields=";
        for (size_t i = 0; i < G.Fields.size(); ++i)
        {
          auto ST = G.Fields[i].ST;
          unsigned Idx = G.Fields[i].FieldIndex;
          if (ST && ST->hasName())
            outs() << ST->getName();
          else
            outs() << "<anon>";
          outs() << "[" << Idx << "]";
          if (i + 1 < G.Fields.size())
            outs() << ", ";
        }
        if (G.Weight != 0.0)
          outs() << " weight=" << G.Weight;
        outs() << "\n";
      }
    }
    (void)MAM;

    return PreservedAnalyses::all();
  }

  // =========================================================
  // Function that traverse a function's loops and collect struct field accesses for each affinity group
  std::vector<CandidateAnalysisPass::AffinityGroup> CandidateAnalysisPass::collectLoopFieldRefs(const Function &F, const FunctionLoopGraph &LoopGraph, const BlockFrequencyInfo *BFI)
  {
    std::vector<AffinityGroup> Groups;
    Groups.reserve(LoopGraph.Nodes.size() + 1);

    // Helper: add unique FieldID to a vector (set semantics per group)
    auto addUnique = [](SmallVector<FieldID, 8> &V, FieldID R)
    {
      if (!R.ST)
        return;
      for (const auto &E : V)
        if (E.ST == R.ST && E.FieldIndex == R.FieldIndex)
          return;
      V.push_back(R);
    };

    // One group per loop node
    for (int Index = 0, E = static_cast<int>(LoopGraph.Nodes.size()); Index != E; ++Index)
    {
      const LoopNode &Node = LoopGraph.Nodes[Index];
      const Loop *L = Node.LoopRef;
      AffinityGroup G;
      G.F = &F;
      G.LoopNodeIndex = Index;

      if (L)
      {
        // Reuse your exact scan pattern from dumpLoopFieldRefs
        // (loads, stores, and GEPs inside the loop’s blocks)
        for (BasicBlock *BB : L->blocks())
        {
          for (Instruction &I : *BB)
          {
            if (auto *LI = dyn_cast<LoadInst>(&I))
            {
              FieldRef Ref = getStructFieldRef(LI->getPointerOperand());
              addUnique(G.Fields, {Ref.ST, Ref.FieldIndex});
            }
            else if (auto *SI = dyn_cast<StoreInst>(&I))
            {
              FieldRef Ref = getStructFieldRef(SI->getPointerOperand());
              addUnique(G.Fields, {Ref.ST, Ref.FieldIndex});
            }
            else if (auto *GEP = dyn_cast<GetElementPtrInst>(&I))
            {
              FieldRef Ref = getStructFieldRef(GEP);
              addUnique(G.Fields, {Ref.ST, Ref.FieldIndex});
            }
          }
        }

        // Optional weight (static): header block frequency
        if (BFI)
        {
          if (const BasicBlock *Hdr = L->getHeader())
            G.Weight = static_cast<double>(BFI->getBlockFreq(Hdr).getFrequency());
        }
      }

      // Only keep non-empty groups for now (you can keep empties if you prefer)
      if (!G.Fields.empty())
        Groups.push_back(std::move(G));
    }

    // (Optional, small scope) Non-loop group: collect fields in BBs with no loop
    // This is easy to add once you thread LoopInfo or a “BB→loop” map here.
    // For now, we focus strictly on loop-based groups per your request.

    return Groups;
  }

  // ================== functions for testing purpose ===================

  // graph that walk through the loops in a function
  void CandidateAnalysisPass::dumpLoopGraph(const Function &F, const FunctionLoopGraph &LoopGraph)
  {
    outs() << "[candidate-analysis] Loop graph for function ";
    if (F.hasName())
      outs() << F.getName();
    else
      F.printAsOperand(outs(), false);
    outs() << '\n';

    if (LoopGraph.Nodes.empty())
    {
      outs() << "  (no loops)\n";
      return;
    }

    for (int Index = 0, E = static_cast<int>(LoopGraph.Nodes.size()); Index != E; ++Index)
    {
      const LoopNode &Node = LoopGraph.Nodes[Index];
      outs() << "  node#" << Index << " parent=";
      if (Node.Parent >= 0)
        outs() << Node.Parent;
      else
        outs() << "none";
      outs() << " header=";
      if (const BasicBlock *Header = Node.LoopRef ? Node.LoopRef->getHeader() : nullptr)
      {
        if (Header->hasName())
          outs() << Header->getName();
        else
          Header->printAsOperand(outs(), false);
      }
      else
      {
        outs() << "<unknown>";
      }
      outs() << " children={";
      for (size_t I = 0; I < Node.Children.size(); ++I)
      {
        outs() << Node.Children[I];
        if (I + 1 < Node.Children.size())
          outs() << ", ";
      }
      outs() << "}\n";
    }
  }

  // walk through all the loops' field references
  void CandidateAnalysisPass::dumpLoopFieldRefs(const Function &F, const FunctionLoopGraph &LoopGraph)
  {
    outs() << "[candidate-analysis] Struct field references per loop in function ";
    if (F.hasName())
      outs() << F.getName();
    else
      F.printAsOperand(outs(), false);
    outs() << '\n';

    if (LoopGraph.Nodes.empty())
    {
      outs() << "  (no loops)\n";
      return;
    }

    for (int Index = 0, E = static_cast<int>(LoopGraph.Nodes.size()); Index != E; ++Index)
    {
      const LoopNode &Node = LoopGraph.Nodes[Index];
      const Loop *L = Node.LoopRef;

      SmallVector<std::pair<StructType *, unsigned>, 8> Fields;
      if (L)
      {
        auto recordPtr = [&](Value *Ptr)
        {
          FieldRef Ref = getStructFieldRef(Ptr);
          if (!Ref.ST)
            return;
          for (const auto &Existing : Fields)
            if (Existing.first == Ref.ST && Existing.second == Ref.FieldIndex)
              return;
          Fields.emplace_back(Ref.ST, Ref.FieldIndex);
        };

        for (BasicBlock *BB : L->blocks())
        {
          for (Instruction &I : *BB)
          {
            if (auto *LI = dyn_cast<LoadInst>(&I))
              recordPtr(LI->getPointerOperand());
            else if (auto *SI = dyn_cast<StoreInst>(&I))
              recordPtr(SI->getPointerOperand());
            else if (auto *GEP = dyn_cast<GetElementPtrInst>(&I))
              recordPtr(GEP);
          }
        }
      }

      outs() << "  loop node#" << Index << ": ";
      if (Fields.empty())
      {
        outs() << "no struct-field GEPs\n";
        continue;
      }

      for (size_t I = 0; I < Fields.size(); ++I)
      {
        StructType *ST = Fields[I].first;
        unsigned FieldIdx = Fields[I].second;
        if (ST && ST->hasName())
          outs() << ST->getName();
        else
          outs() << "<anonymous-struct>";
        outs() << "[" << FieldIdx << "]";
        if (I + 1 < Fields.size())
          outs() << ", ";
      }
      outs() << "\n";
    }
  }

  llvm::PassPluginLibraryInfo getCandidateAnalysisPluginInfo()
  {
    return {LLVM_PLUGIN_API_VERSION, "candidate-analysis", LLVM_VERSION_STRING,
            [](PassBuilder &PB)
            {
              PB.registerPipelineParsingCallback(
                  [](StringRef Name, ModulePassManager &MPM,
                     ArrayRef<PassBuilder::PipelineElement>)
                  {
                    if (Name == "candidate-analysis")
                    {
                      MPM.addPass(CandidateAnalysisPass());
                      return true;
                    }
                    return false;
                  });
            }};
  }

} // namespace candidate

extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo llvmGetPassPluginInfo()
{
  return candidate::getCandidateAnalysisPluginInfo();
}
