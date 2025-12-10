// candidate-analysis/include/candidate-analysis/FieldRef.h
#pragma once
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/Constants.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/SmallVector.h"

namespace candidate
{

    // (StructType*, field index) for direct struct-field GEPs; ST==nullptr if not a match.
    struct FieldRef
    {
        llvm::StructType *ST = nullptr;
        unsigned FieldIndex = ~0u;
    };

    // Returns all struct/field pairs encountered when walking a GEP’s indices
    // from the innermost index back toward the base. Stops once a non-struct
    // index is seen. Empty vector if the value is not a struct GEP. Works with
    // opaque pointers: uses GEP->getSourceElementType().
    inline llvm::SmallVector<FieldRef, 4> getStructFieldRef(const llvm::Value *Ptr)
    {
        using namespace llvm;
        SmallVector<FieldRef, 4> Refs;

        // Handle inst GEPs and const-expr GEPs uniformly
        const Value *V = Ptr;
        while (true)
        {
            if (auto *BC = dyn_cast<BitCastOperator>(V))
            {
                V = BC->getOperand(0);
                continue;
            }
            if (auto *CE = dyn_cast<ConstantExpr>(V))
            {
                if (CE->getOpcode() == Instruction::BitCast)
                {
                    V = CE->getOperand(0);
                    continue;
                }
            }
            break;
        }
        const GEPOperator *GEP = dyn_cast<GEPOperator>(V);
        if (!GEP)
            return Refs;

        // Snapshot the type before each index so we can walk from the end back.
        Type *CurrentTy = GEP->getSourceElementType();
        SmallVector<std::pair<Type *, const Value *>, 8> Steps;
        for (auto IdxIt = GEP->idx_begin(), E = GEP->idx_end(); IdxIt != E; ++IdxIt)
        {
            Steps.push_back({CurrentTy, IdxIt->get()});

            // Advance the working type for the next index.
            if (auto *CurST = dyn_cast<StructType>(CurrentTy))
            {
                if (auto *CI = dyn_cast<ConstantInt>(IdxIt->get()))
                {
                    unsigned FieldIdx = static_cast<unsigned>(CI->getZExtValue());
                    if (FieldIdx < CurST->getNumElements())
                        CurrentTy = CurST->getElementType(FieldIdx);
                    else
                        break; // out of bounds
                }
                else
                {
                    break; // non-constant struct index
                }
            }
            else if (auto *ArrTy = dyn_cast<ArrayType>(CurrentTy))
            {
                CurrentTy = ArrTy->getElementType();
            }
            else if (auto *VecTy = dyn_cast<VectorType>(CurrentTy))
            {
                CurrentTy = VecTy->getElementType();
            }
            else if (isa<PointerType>(CurrentTy))
            {
                break; // opaque pointer: cannot descend further
            }
            else
            {
                break;
            }
        }

        // Walk from innermost outward; record each struct field until ancestry stops.
        for (auto It = Steps.rbegin(), E = Steps.rend(); It != E; ++It)
        {
            if (auto *ST = dyn_cast<StructType>(It->first))
            {
                if (auto *CI = dyn_cast<ConstantInt>(It->second))
                {
                    unsigned FieldIdx = static_cast<unsigned>(CI->getZExtValue());
                    if (FieldIdx < ST->getNumElements())
                        Refs.push_back({ST, FieldIdx});
                }
            }
            else
            {
                break; // stop once we leave struct ancestry
            }
        }

        return Refs;
    }

} // namespace candidate
