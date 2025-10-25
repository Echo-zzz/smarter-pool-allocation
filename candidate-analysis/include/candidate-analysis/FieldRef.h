// candidate-analysis/include/candidate-analysis/FieldRef.h
#pragma once
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Operator.h"

namespace candidate
{

    // (StructType*, field index) for direct struct-field GEPs; ST==nullptr if not a match.
    struct FieldRef
    {
        llvm::StructType *ST = nullptr;
        unsigned FieldIndex = ~0u;
    };

    // Works with opaque pointers: uses GEP->getSourceElementType().
    inline FieldRef getStructFieldRef(const llvm::Value *Ptr)
    {
        using namespace llvm;

        // Handle inst GEPs and const-expr GEPs uniformly
        const Value *V = Ptr->stripPointerCasts();
        const GEPOperator *GEP = dyn_cast<GEPOperator>(V);
        if (!GEP)
            return {};

        // The source element type reflects what we're indexing into (with opaque ptrs)
        Type *ElemTy = GEP->getSourceElementType();
        auto *ST = dyn_cast<StructType>(ElemTy);
        if (!ST)
            return {};

        // For struct GEPs, the field index is a constant in operand #2 (after the leading 0).
        // Be robust and scan the gep indices
        auto It = GEP->idx_begin();
        if (It == GEP->idx_end())
            return {};
        ++It; // skip leading aggregate index (usually 0)
        if (It == GEP->idx_end())
            return {};
        auto *CIdx = dyn_cast<ConstantInt>(It->get());
        if (!CIdx)
            return {};
        return {ST, static_cast<unsigned>(CIdx->getZExtValue())};
    }

} // namespace candidate
