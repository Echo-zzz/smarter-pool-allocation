// candidate-analysis/include/candidate-analysis/ReportUtils.h
#pragma once

#include <string>

#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"

namespace llvm
{
  class StructType;
}

namespace candidate
{

  /// Shared command-line option that dictates where reports (static or dynamic)
  /// should be written.
  extern llvm::cl::opt<std::string> CandidateAnalysisOutputDir;

  /// Replace characters unsuitable for filesystem paths with underscores.
  std::string sanitizeForFilename(llvm::StringRef Name);

  /// Produces a stable label for a struct type (named types keep their name,
  /// anonymous ones record their pointer).
  std::string getStructLabel(const llvm::StructType *ST);

} // namespace candidate

