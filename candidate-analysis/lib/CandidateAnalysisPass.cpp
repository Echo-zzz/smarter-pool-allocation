#include "candidate-analysis/CandidateAnalysisPass.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/LoopInfo.h"

#define DEBUG_TYPE "candidate-analysis"

using namespace llvm;

namespace candidate
{

  PreservedAnalyses CandidateAnalysisPass::run(Module &M, ModuleAnalysisManager &MAM)
  {
    LLVM_DEBUG(dbgs() << "[candidate-analysis] Visiting module: " << M.getName() << '\n');

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
    }
    (void)MAM;

    return PreservedAnalyses::all();
  }

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
