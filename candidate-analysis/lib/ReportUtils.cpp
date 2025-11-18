#include "candidate-analysis/ReportUtils.h"

#include "llvm/IR/Type.h"
#include "llvm/Support/raw_ostream.h"

#include <cctype>

using namespace llvm;

namespace candidate
{

  cl::opt<std::string> CandidateAnalysisOutputDir(
      "candidate-analysis-output-dir",
      cl::desc("Directory to store candidate-analysis YAML reports"),
      cl::init("candidate-analysis-report"));

  std::string sanitizeForFilename(StringRef Name)
  {
    if (Name.empty())
      return "module";
    std::string Clean;
    Clean.reserve(Name.size());
    for (char C : Name)
    {
      unsigned char UC = static_cast<unsigned char>(C);
      if (std::isalnum(UC) || C == '-' || C == '_')
        Clean.push_back(C);
      else
        Clean.push_back('_');
    }
    return Clean;
  }

  std::string getStructLabel(const StructType *ST)
  {
    if (!ST)
      return "<null>";
    if (ST->hasName())
      return ST->getName().str();
    std::string Label;
    raw_string_ostream OS(Label);
    OS << "anon@" << ST;
    return OS.str();
  }

} // namespace candidate

