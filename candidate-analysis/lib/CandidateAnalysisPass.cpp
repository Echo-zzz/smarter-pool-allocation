#include "candidate-analysis/CandidateAnalysisPass.h"

#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "candidate-analysis"

using namespace llvm;

namespace candidate {

PreservedAnalyses CandidateAnalysisPass::run(Module &M, ModuleAnalysisManager &MAM) {
  LLVM_DEBUG(dbgs() << "[candidate-analysis] Visiting module: " << M.getName() << '\n');

  // TODO: Implement loop-affinity analysis here.
  (void)MAM;

  return PreservedAnalyses::all();
}

llvm::PassPluginLibraryInfo getCandidateAnalysisPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "candidate-analysis", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager &MPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "candidate-analysis") {
                    MPM.addPass(CandidateAnalysisPass());
                    return true;
                  }
                  return false;
                });
          }};
}

} // namespace candidate

extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return candidate::getCandidateAnalysisPluginInfo();
}
