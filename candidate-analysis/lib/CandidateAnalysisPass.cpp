#include "candidate-analysis/CandidateAnalysisPass.h"
#include "candidate-analysis/FieldRef.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/CFG.h"
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
    std::vector<AffinityGroup> AllGroups;
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

      DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);

      // // function calls for testing purpose
      // dumpLoopGraph(F, LoopGraph);
      // dumpLoopFieldRefs(F, LoopGraph, LI, &DT);

      // const BlockFrequencyInfo *BFI = nullptr; // (thread real BFI later)
      const BlockFrequencyInfo &BFI = FAM.getResult<BlockFrequencyAnalysis>(F);
      auto Groups = collectLoopFieldRefs(F, LoopGraph, LI, &DT, &BFI);
      AllGroups.insert(AllGroups.end(), Groups.begin(), Groups.end());

      // dumpAffinityGroups(Groups);
    }
    auto MergedGroups = mergeIdenticalGroupsByKey(AllGroups);
    // debug-print the merged result
    dumpMergedGroups(MergedGroups);
    (void)MAM;

    return PreservedAnalyses::all();
  }

  // =========================================================
  // Function that traverse a function's loops and collect struct field accesses for each affinity group
  std::vector<CandidateAnalysisPass::AffinityGroup> CandidateAnalysisPass::collectLoopFieldRefs(const Function &F, const FunctionLoopGraph &LoopGraph, const LoopInfo &LI, const DominatorTree *DT, const BlockFrequencyInfo *BFI)
  {
    std::vector<AffinityGroup> Groups;
    Groups.reserve(LoopGraph.Nodes.size() + 1);

    // Helper: bump field frequency within a group.
    auto bumpField = [](AffinityGroup &Group, FieldID R)
    {
      if (!R.ST)
        return;
      auto It = Group.Fields.find(R);
      if (It == Group.Fields.end())
        Group.Fields.insert({R, 1u});
      else
        ++It->second;
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
        const BasicBlock *Header = L->getHeader();
        SmallPtrSet<BasicBlock *, 16> Visited;
        SmallVector<BasicBlock *, 16> Worklist;
        SmallVector<BasicBlock *, 16> BodyBlocks;

        auto enqueueBlock = [&](BasicBlock *BB)
        {
          if (!BB)
            return;
          if (Visited.insert(BB).second)
          {
            Worklist.push_back(BB);
            BodyBlocks.push_back(BB);
          }
        };

        // Always enqueue the blocks LoopInfo already associates with the loop.
        for (BasicBlock *BB : L->blocks())
        {
          if (LI.getLoopFor(BB) != L)
            continue; // owned by a subloop; let child handle it
          enqueueBlock(BB);
        }

        // LoopInfo::contains can report additional body blocks even when the loop
        // is not in canonical form (e.g. -O0 IR), so seed them as well.
        for (const BasicBlock &BBRef : F)
        {
          BasicBlock *BB = const_cast<BasicBlock *>(&BBRef);
          if (L->contains(BB) && LI.getLoopFor(BB) == L)
            enqueueBlock(BB);
        }

        if (BodyBlocks.empty() && Header)
          enqueueBlock(const_cast<BasicBlock *>(Header));

        // Reachability walk: stay inside the loop, skip subloops, and ensure the
        // header dominates successors before enqueuing them. This gives us a
        // robust set of body blocks without accidentally including post-loop code.
        while (!Worklist.empty())
        {
          BasicBlock *BB = Worklist.pop_back_val();
          for (BasicBlock *Succ : successors(BB))
          {
            if (!Succ)
              continue;

            if (!L->contains(Succ) || LI.getLoopFor(Succ) != L)
              continue;

            // Skip if this block naturally belongs to a child loop.
            if (Loop *SuccLoop = LI.getLoopFor(Succ))
            {
              if (SuccLoop != L && L->contains(SuccLoop))
                continue;
            }

            if (DT && Header && !DT->dominates(Header, Succ))
              continue;

            enqueueBlock(Succ);
          }
        }

        // Collect struct field references from every block determined to be part
        // of the loop body.
        for (BasicBlock *BB : BodyBlocks)
        {
          for (Instruction &I : *BB)
          {
            if (auto *Load = dyn_cast<LoadInst>(&I))
            {
              FieldRef Ref = getStructFieldRef(Load->getPointerOperand());
              bumpField(G, {Ref.ST, Ref.FieldIndex});
            }
            else if (auto *SI = dyn_cast<StoreInst>(&I))
            {
              FieldRef Ref = getStructFieldRef(SI->getPointerOperand());
              bumpField(G, {Ref.ST, Ref.FieldIndex});
            }
            else if (auto *GEP = dyn_cast<GetElementPtrInst>(&I))
            {
              FieldRef Ref = getStructFieldRef(GEP);
              bumpField(G, {Ref.ST, Ref.FieldIndex});
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

    // Collect field references for non-loop basic blocks into a pseudo group.
    AffinityGroup NonLoopGroup;
    NonLoopGroup.F = &F;
    NonLoopGroup.LoopNodeIndex = -1; // denote "routine entry" pseudo-node

    if (BFI)
      NonLoopGroup.Weight = 0.0;

    for (const BasicBlock &BBRef : F)
    {
      const BasicBlock *BB = &BBRef;
      if (LI.getLoopFor(BB))
        continue;

      if (BFI)
        NonLoopGroup.Weight += static_cast<double>(BFI->getBlockFreq(BB).getFrequency());

      for (const Instruction &I : BBRef)
      {
        if (const auto *Load = dyn_cast<LoadInst>(&I))
        {
          FieldRef Ref = getStructFieldRef(Load->getPointerOperand());
          bumpField(NonLoopGroup, {Ref.ST, Ref.FieldIndex});
        }
        else if (const auto *SI = dyn_cast<StoreInst>(&I))
        {
          FieldRef Ref = getStructFieldRef(SI->getPointerOperand());
          bumpField(NonLoopGroup, {Ref.ST, Ref.FieldIndex});
        }
        else if (const auto *GEP = dyn_cast<GetElementPtrInst>(&I))
        {
          FieldRef Ref = getStructFieldRef(GEP);
          bumpField(NonLoopGroup, {Ref.ST, Ref.FieldIndex});
        }
      }
    }

    if (!NonLoopGroup.Fields.empty())
      Groups.push_back(std::move(NonLoopGroup));

    return Groups;
  }

  // Canonicalize a group into a stable, sorted key and accumulate weight per key.
  std::vector<CandidateAnalysisPass::AffinityGroup>
  CandidateAnalysisPass::mergeIdenticalGroupsByKey(const std::vector<AffinityGroup> &Groups)
  {
    using namespace llvm;
    StringMap<size_t> KeyToIndex;
    std::vector<AffinityGroup> Result;

    SmallString<256> KeyBuf; // reuse buffer to avoid reallocating per group
    for (const auto &G : Groups)
    {
      // Build a canonical vector of (StructType*, FieldIndex)
      SmallVector<std::pair<FieldID, unsigned>, 8> Vec;
      Vec.reserve(G.Fields.size());
      for (const auto &KV : G.Fields)
        Vec.emplace_back(KV.first, KV.second);

      // Sort to make the key order-independent.
      llvm::sort(Vec, [](const auto &A, const auto &B)
                 {
      if (A.first.ST != B.first.ST) return A.first.ST < B.first.ST;
      return A.first.FieldIndex < B.first.FieldIndex; });

      // Serialize to a compact string key (pointer prints are stable within a run)
      KeyBuf.clear();
      {
        raw_svector_ostream OS(KeyBuf);
        for (const auto &E : Vec)
        {
          OS << E.first.ST << '.' << E.first.FieldIndex << ':' << E.second << ';';
        }
      }

      // Lookup/insert canonical key in the accumulator.
      auto InsertRes = KeyToIndex.try_emplace(KeyBuf, Result.size());
      if (InsertRes.second)
      {
        // First time we see this field multiset; create a new merged group.
        Result.emplace_back();
        AffinityGroup &Merged = Result.back();
        Merged.F = nullptr;
        Merged.LoopNodeIndex = -1;
        Merged.Weight = G.Weight;
        for (const auto &Entry : Vec)
          Merged.Fields.insert({Entry.first, Entry.second});
      }
      else
      {
        // Merge into existing entry: accumulate weight and field counts.
        AffinityGroup &Existing = Result[InsertRes.first->second];
        Existing.Weight += G.Weight;
        for (const auto &Entry : Vec)
        {
          auto It = Existing.Fields.find(Entry.first);
          if (It == Existing.Fields.end())
            Existing.Fields.insert({Entry.first, Entry.second});
          else
            It->second += Entry.second;
        }
      }
    }

    return Result;
  }

  void CandidateAnalysisPass::dumpAffinityGroups(const std::vector<AffinityGroup> &Groups)
  {
    for (const auto &G : Groups)
    {
      outs() << "[candidate-analysis] group loop#" << G.LoopNodeIndex << " fields=";
      SmallVector<std::pair<FieldID, unsigned>, 8> Entries;
      Entries.reserve(G.Fields.size());
      for (const auto &KV : G.Fields)
        Entries.emplace_back(KV.first, KV.second);
      llvm::sort(Entries, [](const auto &A, const auto &B)
                 {
      if (A.first.ST != B.first.ST) return A.first.ST < B.first.ST;
      return A.first.FieldIndex < B.first.FieldIndex; });

      for (size_t I = 0; I < Entries.size(); ++I)
      {
        const FieldID &FID = Entries[I].first;
        unsigned Count = Entries[I].second;
        auto *ST = FID.ST;
        unsigned Idx = FID.FieldIndex;
        if (ST && ST->hasName())
          outs() << ST->getName();
        else
          outs() << "<anon>";
        outs() << "[" << Idx << "]";
        if (Count > 1)
          outs() << "x" << Count;
        if (I + 1 < Entries.size())
          outs() << ", ";
      }
      if (G.Weight != 0.0)
        outs() << " weight=" << G.Weight;
      outs() << "\n";
    }
  }

  void CandidateAnalysisPass::dumpMergedGroups(const std::vector<AffinityGroup> &MergedGroups)
  {
    for (const auto &G : MergedGroups)
    {
      outs() << "[candidate-analysis] merged fields={";
      SmallVector<std::pair<FieldID, unsigned>, 8> Entries;
      Entries.reserve(G.Fields.size());
      for (const auto &KV : G.Fields)
        Entries.emplace_back(KV.first, KV.second);
      llvm::sort(Entries, [](const auto &A, const auto &B)
                 {
      if (A.first.ST != B.first.ST) return A.first.ST < B.first.ST;
      return A.first.FieldIndex < B.first.FieldIndex; });

      for (size_t I = 0; I < Entries.size(); ++I)
      {
        const FieldID &Field = Entries[I].first;
        unsigned Count = Entries[I].second;
        if (Field.ST && Field.ST->hasName())
          outs() << Field.ST->getName();
        else
          outs() << "<anon>";
        outs() << "[" << Field.FieldIndex << "]";
        if (Count > 1)
          outs() << "x" << Count;
        if (I + 1 < Entries.size())
          outs() << ", ";
      }
      outs() << "} total_weight=" << G.Weight << "\n";
    }
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
  void CandidateAnalysisPass::dumpLoopFieldRefs(const Function &F, const FunctionLoopGraph &LoopGraph,
                                                const LoopInfo &LI, const DominatorTree *DT)
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

    std::vector<AffinityGroup> Groups = collectLoopFieldRefs(F, LoopGraph, LI, DT, /*BFI=*/nullptr);

    DenseMap<int, SmallVector<std::pair<FieldID, unsigned>, 4>> FieldsByLoop;
    for (const auto &G : Groups)
    {
      SmallVector<std::pair<FieldID, unsigned>, 4> Local;
      for (const auto &KV : G.Fields)
        Local.push_back(KV);
      FieldsByLoop[G.LoopNodeIndex] = std::move(Local);
    }

    for (int Index = 0, E = static_cast<int>(LoopGraph.Nodes.size()); Index != E; ++Index)
    {
      outs() << "  loop node#" << Index << ": ";
      auto It = FieldsByLoop.find(Index);
      if (It == FieldsByLoop.end() || It->second.empty())
      {
        outs() << "no struct-field GEPs\n";
        continue;
      }

      const auto &Fields = It->second;
      for (size_t I = 0; I < Fields.size(); ++I)
      {
        StructType *ST = Fields[I].first.ST;
        unsigned FieldIdx = Fields[I].first.FieldIndex;
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
