#include "canaidate-analysis/CanaidateAnalysisPass.h"

#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "canaidate-analysis"

using namespace llvm;

namespace canaidate {

PreservedAnalyses CanaidateAnalysisPass::run(Module &M, ModuleAnalysisManager &MAM) {
  LLVM_DEBUG(dbgs() << "[canaidate-analysis] Visiting module: " << M.getName() << '\n');

  // TODO: Implement loop-affinity analysis here.
  (void)MAM;

  return PreservedAnalyses::all();
}

llvm::PassPluginLibraryInfo getCanaidateAnalysisPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "canaidate-analysis", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager &MPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "canaidate-analysis") {
                    MPM.addPass(CanaidateAnalysisPass());
                    return true;
                  }
                  return false;
                });
          }};
}

} // namespace canaidate

extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return canaidate::getCanaidateAnalysisPluginInfo();
}
