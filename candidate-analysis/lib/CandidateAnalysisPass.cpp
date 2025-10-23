#include "candidate-analysis/CandidateAnalysisPass.h"

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

      // walk the natural loops
      for (Loop *Root : LI)
      {
        SmallVector<Loop *, 8> stack{Root};
        while (!stack.empty())
        {
          Loop *L = stack.pop_back_val();
          for (BasicBlock *BB : L->blocks())
          {
            outs() << "[candidate-analysis] Loop block: ";
            if (BB->hasName())
              outs() << BB->getName();
            else
              BB->printAsOperand(outs(), false);
            outs() << '\n';
          }

          for (Loop *SL : L->getSubLoops())
            stack.push_back(SL);
        }
      }
    }
    (void)MAM;

    return PreservedAnalyses::all();
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
