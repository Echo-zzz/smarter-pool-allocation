set(CANDIDATE_ANALYSIS_SOURCES
    CandidateAnalysisPass.cpp
)

add_library(CandidateAnalysis MODULE ${CANDIDATE_ANALYSIS_SOURCES})

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
