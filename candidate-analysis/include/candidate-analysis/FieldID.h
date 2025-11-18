// candidate-analysis/include/candidate-analysis/FieldID.h
#pragma once

#include "llvm/IR/DerivedTypes.h"
#include "llvm/ADT/DenseMapInfo.h"
#include <cstdint>

namespace candidate
{

  /// Canonical identity for a struct field in the IR: the StructType pointer
  /// combined with the field index. This mirrors the notion used by the static
  /// candidate analysis so the profiler can key off the same identifiers.
  struct FieldID
  {
    llvm::StructType *ST = nullptr;
    unsigned FieldIndex = ~0u;

    bool operator==(const FieldID &O) const
    {
      return ST == O.ST && FieldIndex == O.FieldIndex;
    }
  };

  /// DenseMap adaptor so we can use FieldID as a key without rewriting the
  /// hashing logic in every pass that needs it.
  struct FieldIDInfo
  {
    static FieldID getEmptyKey()
    {
      return {reinterpret_cast<llvm::StructType *>(-1), ~0u};
    }

    static FieldID getTombstoneKey()
    {
      return {reinterpret_cast<llvm::StructType *>(-2), ~0u};
    }

    static unsigned getHashValue(const FieldID &F)
    {
      return static_cast<unsigned>((reinterpret_cast<uintptr_t>(F.ST) >> 3) ^
                                   (F.FieldIndex * 1315423911u));
    }

    static bool isEqual(const FieldID &LHS, const FieldID &RHS)
    {
      return LHS == RHS;
    }
  };

} // namespace candidate

