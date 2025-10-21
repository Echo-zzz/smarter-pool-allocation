set(CANDIDATE_ANALYSIS_SOURCES
    CandidateAnalysisPass.cpp
)

add_library(CandidateAnalysis MODULE ${CANDIDATE_ANALYSIS_SOURCES})

# Prefer the monolithic shared LLVM library to avoid duplicate cl::opts
if (LLVM_LINK_LLVM_DYLIB)
  target_link_libraries(CandidateAnalysis PRIVATE LLVM)
else()
  message(WARNING
    "Falling back to component libs (LLVMCore, LLVMAnalysis, LLVMSupport). "
    "This can cause duplicate command-line option registrations when loaded by opt.")
  target_link_libraries(CandidateAnalysis PRIVATE LLVMCore LLVMAnalysis LLVMSupport)
endif()

# LLVM expects plugin targets to be built with PIC
set_target_properties(CandidateAnalysis PROPERTIES
    OUTPUT_NAME "CandidateAnalysis"
)

target_include_directories(CandidateAnalysis
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(CandidateAnalysis
    PRIVATE
        LLVMCore
        LLVMAnalysis
        LLVMSupport
)

# Give users a hint if they try to build static libraries
if (NOT LLVM_ENABLE_RTTI)
  target_compile_definitions(CandidateAnalysis PRIVATE CANDIDATE_NO_RTTI)
endif()
