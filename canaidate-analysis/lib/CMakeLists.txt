set(CANAIDATE_ANALYSIS_SOURCES
    CanaidateAnalysisPass.cpp
)

add_library(CanaidateAnalysis MODULE ${CANAIDATE_ANALYSIS_SOURCES})

# LLVM expects plugin targets to be built with PIC
set_target_properties(CanaidateAnalysis PROPERTIES
    OUTPUT_NAME "CanaidateAnalysis"
)

target_include_directories(CanaidateAnalysis
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(CanaidateAnalysis
    PRIVATE
        LLVMCore
        LLVMAnalysis
        LLVMSupport
)

# Give users a hint if they try to build static libraries
if (NOT LLVM_ENABLE_RTTI)
  target_compile_definitions(CanaidateAnalysis PRIVATE CANAIDATE_NO_RTTI)
endif()
