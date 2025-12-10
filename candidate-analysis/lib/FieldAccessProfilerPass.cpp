#include "candidate-analysis/FieldAccessProfilerPass.h"

#include "candidate-analysis/FieldRef.h"
#include "candidate-analysis/ReportUtils.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/Alignment.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/YAMLTraits.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"

#include <algorithm>
#include <numeric>
#include <string>
#include <vector>

using namespace llvm;

namespace candidate
{

  namespace
  {
    struct FieldInfoRecord
    {
      FieldID ID;
      std::string StructLabel;
      std::string EscapedLabel;
    };

    struct InstrumentationSite
    {
      Instruction *Inst = nullptr;
      unsigned CounterIndex = 0;
    };

    Constant *createStringLiteral(Module &M, StringRef Value, const Twine &Name)
    {
      LLVMContext &Ctx = M.getContext();
      Constant *Data = ConstantDataArray::getString(Ctx, Value, true);
      auto *GV = new GlobalVariable(M, Data->getType(), true, GlobalValue::PrivateLinkage, Data, Name);
      GV->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);
      GV->setAlignment(Align(1));
      Constant *Zero32 = ConstantInt::get(Type::getInt32Ty(Ctx), 0);
      SmallVector<llvm::Value *, 2> Indices;
      Indices.push_back(Zero32);
      Indices.push_back(Zero32);
      return ConstantExpr::getInBoundsGetElementPtr(Data->getType(), GV, Indices);
    }

    FunctionCallee getOrInsertMkdir(Module &M)
    {
      LLVMContext &Ctx = M.getContext();
      Type *I32Ty = Type::getInt32Ty(Ctx);
      Type *I8PtrTy = PointerType::get(Type::getInt8Ty(Ctx), 0);
      FunctionType *FnTy = FunctionType::get(I32Ty, {I8PtrTy, I32Ty}, false);
      return M.getOrInsertFunction("mkdir", FnTy);
    }

    FunctionCallee getOrInsertFopen(Module &M)
    {
      LLVMContext &Ctx = M.getContext();
      PointerType *I8PtrTy = PointerType::get(Type::getInt8Ty(Ctx), 0);
      FunctionType *FnTy = FunctionType::get(I8PtrTy, {I8PtrTy, I8PtrTy}, false);
      return M.getOrInsertFunction("fopen", FnTy);
    }

    FunctionCallee getOrInsertFprintf(Module &M)
    {
      LLVMContext &Ctx = M.getContext();
      Type *I32Ty = Type::getInt32Ty(Ctx);
      PointerType *I8PtrTy = PointerType::get(Type::getInt8Ty(Ctx), 0);
      FunctionType *FnTy = FunctionType::get(I32Ty, {I8PtrTy, I8PtrTy}, true);
      return M.getOrInsertFunction("fprintf", FnTy);
    }

    FunctionCallee getOrInsertFclose(Module &M)
    {
      LLVMContext &Ctx = M.getContext();
      Type *I32Ty = Type::getInt32Ty(Ctx);
      PointerType *I8PtrTy = PointerType::get(Type::getInt8Ty(Ctx), 0);
      FunctionType *FnTy = FunctionType::get(I32Ty, {I8PtrTy}, false);
      return M.getOrInsertFunction("fclose", FnTy);
    }

    FunctionCallee getOrInsertAtexit(Module &M)
    {
      LLVMContext &Ctx = M.getContext();
      Type *I32Ty = Type::getInt32Ty(Ctx);
      Type *VoidTy = Type::getVoidTy(Ctx);
      FunctionType *CallbackTy = FunctionType::get(VoidTy, false);
      PointerType *CallbackPtrTy = PointerType::getUnqual(CallbackTy);
      FunctionType *FnTy = FunctionType::get(I32Ty, {CallbackPtrTy}, false);
      return M.getOrInsertFunction("atexit", FnTy);
    }

    void insertCounterIncrement(Instruction &I, unsigned CounterIndex,
                                GlobalVariable *Counters, ArrayType *CountersTy)
    {
      LLVMContext &Ctx = I.getModule()->getContext();
      Type *Int64Ty = Type::getInt64Ty(Ctx);
      Constant *Zero = ConstantInt::get(Int64Ty, 0);
      Constant *Index = ConstantInt::get(Int64Ty, CounterIndex);

      IRBuilder<> Builder(I.getParent());
      auto InsertIt = std::next(BasicBlock::iterator(I));
      Builder.SetInsertPoint(I.getParent(), InsertIt);

      Value *Ptr = Builder.CreateInBoundsGEP(CountersTy, Counters, {Zero, Index}, "field_counter_ptr");
      Builder.CreateAtomicRMW(AtomicRMWInst::Add, Ptr, ConstantInt::get(Int64Ty, 1),
                              MaybeAlign(8), AtomicOrdering::Monotonic);
    }

    void buildRuntimeReporting(Module &M,
                               ArrayType *CountersTy,
                               GlobalVariable *CountersGV,
                                ArrayType *FieldInfoArrayTy,
                                GlobalVariable *FieldInfoGV,
                                ArrayType *OrderArrayTy,
                                GlobalVariable *OrderGV,
                                Constant *EscapedModuleLabelStr,
                                Constant *RootDirStr,
                                Constant *ModuleDirStr,
                                Constant *ReportPathStr)
    {
      LLVMContext &Ctx = M.getContext();
      Type *VoidTy = Type::getVoidTy(Ctx);
      Type *Int32Ty = Type::getInt32Ty(Ctx);
      Type *Int64Ty = Type::getInt64Ty(Ctx);
      PointerType *I8PtrTy = PointerType::get(Type::getInt8Ty(Ctx), 0);
      Constant *Zero64 = ConstantInt::get(Int64Ty, 0);
      Constant *ModeStr = createStringLiteral(M, "w", "__field_profiler_w_mode");
      Constant *HeaderFmt = createStringLiteral(M, "module: \"%s\"\n", "__field_profiler_header_fmt");
      Constant *FieldsFmt = createStringLiteral(M, "fields:\n", "__field_profiler_fields_fmt");
      Constant *EntryFmt = createStringLiteral(M, "  - struct: \"%s\"\n    index: %u\n    count: %llu\n",
                                               "__field_profiler_entry_fmt");
      Constant *EmptyFmt = createStringLiteral(M, "fields: []\n", "__field_profiler_empty_fmt");
      Constant *LineBreak = createStringLiteral(M, "\n", "__field_profiler_newline");

      auto *FieldInfoTy = cast<StructType>(FieldInfoArrayTy->getElementType());

      Function *EmitFn = Function::Create(FunctionType::get(VoidTy, false),
                                          GlobalValue::InternalLinkage,
                                          "__field_profiler_emit", &M);

      BasicBlock *EntryBB = BasicBlock::Create(Ctx, "entry", EmitFn);
      BasicBlock *WriteHeaderBB = BasicBlock::Create(Ctx, "write.header", EmitFn);
      BasicBlock *LoopCheckBB = BasicBlock::Create(Ctx, "loop.check", EmitFn);
      BasicBlock *LoopBodyBB = BasicBlock::Create(Ctx, "loop.body", EmitFn);
      BasicBlock *AfterLoopBB = BasicBlock::Create(Ctx, "after.loop", EmitFn);
      BasicBlock *ExitBB = BasicBlock::Create(Ctx, "exit", EmitFn);

      IRBuilder<> Builder(EntryBB);
      AllocaInst *FileAlloca = Builder.CreateAlloca(I8PtrTy, nullptr, "file.ptr");
      AllocaInst *IndexAlloca = Builder.CreateAlloca(Int64Ty, nullptr, "index");
      Builder.CreateCall(getOrInsertMkdir(M), {RootDirStr, ConstantInt::get(Int32Ty, 0755)});
      Builder.CreateCall(getOrInsertMkdir(M), {ModuleDirStr, ConstantInt::get(Int32Ty, 0755)});

      Value *FileHandle = Builder.CreateCall(getOrInsertFopen(M), {ReportPathStr, ModeStr}, "file");
      Builder.CreateStore(FileHandle, FileAlloca);
      Builder.CreateStore(ConstantInt::get(Int64Ty, 0), IndexAlloca);

      Value *IsNull = Builder.CreateICmpEQ(FileHandle, ConstantPointerNull::get(I8PtrTy));
      Builder.CreateCondBr(IsNull, ExitBB, WriteHeaderBB);

      IRBuilder<> HeaderBuilder(WriteHeaderBB);
      Value *OpenedFile = HeaderBuilder.CreateLoad(I8PtrTy, FileAlloca, "file.handle");
      HeaderBuilder.CreateCall(getOrInsertFprintf(M), {OpenedFile, HeaderFmt, EscapedModuleLabelStr});

      uint64_t FieldCount = CountersTy->getNumElements();
      if (FieldCount == 0)
      {
        HeaderBuilder.CreateCall(getOrInsertFprintf(M), {OpenedFile, EmptyFmt});
        HeaderBuilder.CreateBr(AfterLoopBB);
      }
      else
      {
        HeaderBuilder.CreateCall(getOrInsertFprintf(M), {OpenedFile, FieldsFmt});
        HeaderBuilder.CreateBr(LoopCheckBB);
      }

      IRBuilder<> LoopCheckBuilder(LoopCheckBB);
      Value *CurrentIndex = LoopCheckBuilder.CreateLoad(Int64Ty, IndexAlloca, "idx");
      Value *FieldCountVal = ConstantInt::get(Int64Ty, FieldCount);
      Value *ShouldContinue = LoopCheckBuilder.CreateICmpULT(CurrentIndex, FieldCountVal);
      LoopCheckBuilder.CreateCondBr(ShouldContinue, LoopBodyBB, AfterLoopBB);

      IRBuilder<> BodyBuilder(LoopBodyBB);
      Value *LoopFile = BodyBuilder.CreateLoad(I8PtrTy, FileAlloca, "file.loop");
      Value *OrderPtr = BodyBuilder.CreateInBoundsGEP(OrderArrayTy, OrderGV, {Zero64, CurrentIndex});
      Value *CounterIndex32 = BodyBuilder.CreateLoad(Int32Ty, OrderPtr, "counter.index.32");
      Value *CounterIndex64 = BodyBuilder.CreateZExt(CounterIndex32, Int64Ty, "counter.index");

      Value *FieldInfoPtr = BodyBuilder.CreateInBoundsGEP(FieldInfoArrayTy, FieldInfoGV,
                                                          {Zero64, CounterIndex64}, "field.info.ptr");
      Value *StructNamePtr = BodyBuilder.CreateStructGEP(FieldInfoTy, FieldInfoPtr, 0, "struct.name.ptr");
      Value *StructName = BodyBuilder.CreateLoad(I8PtrTy, StructNamePtr, "struct.name");
      Value *FieldIndexPtr = BodyBuilder.CreateStructGEP(FieldInfoTy, FieldInfoPtr, 1, "field.index.ptr");
      Value *FieldIndex = BodyBuilder.CreateLoad(Int32Ty, FieldIndexPtr, "field.index");

      Value *CounterPtr = BodyBuilder.CreateInBoundsGEP(CountersTy, CountersGV,
                                                        {Zero64, CounterIndex64}, "counter.ptr");
      Value *CounterVal = BodyBuilder.CreateLoad(Int64Ty, CounterPtr, "counter.val");

      BodyBuilder.CreateCall(getOrInsertFprintf(M),
                             {LoopFile, EntryFmt, StructName, FieldIndex, CounterVal});

      Value *NextIndex = BodyBuilder.CreateAdd(CurrentIndex, ConstantInt::get(Int64Ty, 1), "next.idx");
      BodyBuilder.CreateStore(NextIndex, IndexAlloca);
      BodyBuilder.CreateBr(LoopCheckBB);

      IRBuilder<> AfterBuilder(AfterLoopBB);
      Value *FileAfter = AfterBuilder.CreateLoad(I8PtrTy, FileAlloca, "file.after");
      AfterBuilder.CreateCall(getOrInsertFprintf(M), {FileAfter, LineBreak});
      AfterBuilder.CreateCall(getOrInsertFclose(M), {FileAfter});
      AfterBuilder.CreateBr(ExitBB);

      IRBuilder<> ExitBuilder(ExitBB);
      ExitBuilder.CreateRetVoid();

      Function *RegisterFn = Function::Create(FunctionType::get(VoidTy, false),
                                              GlobalValue::InternalLinkage,
                                              "__field_profiler_register", &M);
      BasicBlock *RegEntry = BasicBlock::Create(Ctx, "entry", RegisterFn);
      IRBuilder<> RegBuilder(RegEntry);
      RegBuilder.CreateCall(getOrInsertAtexit(M), {EmitFn});
      RegBuilder.CreateRetVoid();
      appendToGlobalCtors(M, RegisterFn, 0);
    }

  } // namespace

  PreservedAnalyses FieldAccessProfilerPass::run(Module &M, ModuleAnalysisManager &)
  {
    DenseMap<FieldID, unsigned, FieldIDInfo> FieldToIndex;
    std::vector<FieldInfoRecord> FieldInfos;
    std::vector<InstrumentationSite> Sites;

    auto getOrCreateIndex = [&](FieldID ID) -> unsigned
    {
      auto It = FieldToIndex.find(ID);
      if (It != FieldToIndex.end())
        return It->second;

      unsigned NewIndex = FieldInfos.size();
      FieldToIndex.insert({ID, NewIndex});
      FieldInfoRecord Info;
      Info.ID = ID;
      Info.StructLabel = getStructLabel(ID.ST);
      Info.EscapedLabel = yaml::escape(Info.StructLabel);
      FieldInfos.push_back(std::move(Info));
      return NewIndex;
    };

    auto track = [&](Instruction &I, FieldRef Ref)
    {
      if (!Ref.ST)
        return;
      FieldID ID{Ref.ST, Ref.FieldIndex};
      unsigned Index = getOrCreateIndex(ID);
      Sites.push_back({&I, Index});
    };

    for (Function &F : M)
    {
      if (F.isDeclaration())
        continue;
      for (BasicBlock &BB : F)
      {
        for (Instruction &I : BB)
        {
          if (auto *Load = dyn_cast<LoadInst>(&I))
          {
            auto Refs = getStructFieldRef(Load->getPointerOperand());
            for (const auto &Ref : Refs)
              track(I, Ref);
          }
          else if (auto *Store = dyn_cast<StoreInst>(&I))
          {
            auto Refs = getStructFieldRef(Store->getPointerOperand());
            for (const auto &Ref : Refs)
              track(I, Ref);
          }
          else if (auto *GEP = dyn_cast<GetElementPtrInst>(&I))
          {
            auto Refs = getStructFieldRef(GEP);
            for (const auto &Ref : Refs)
              track(I, Ref);
          }
        }
      }
    }

    if (FieldInfos.empty())
      return PreservedAnalyses::all();

    LLVMContext &Ctx = M.getContext();
    Type *Int64Ty = Type::getInt64Ty(Ctx);
    ArrayType *CountersTy = ArrayType::get(Int64Ty, FieldInfos.size());
    auto *CountersGV = new GlobalVariable(M, CountersTy, false, GlobalValue::InternalLinkage,
                                          Constant::getNullValue(CountersTy),
                                          "__field_profiler_counters");
    CountersGV->setAlignment(Align(8));

    for (const auto &Site : Sites)
      insertCounterIncrement(*Site.Inst, Site.CounterIndex, CountersGV, CountersTy);

    StructType *FieldInfoTy = StructType::create(Ctx, "candidate.field.profiler.info");
    FieldInfoTy->setBody({PointerType::get(Type::getInt8Ty(Ctx), 0), Type::getInt32Ty(Ctx)});
    ArrayType *FieldInfoArrayTy = ArrayType::get(FieldInfoTy, FieldInfos.size());

    std::vector<Constant *> FieldInfoConsts;
    FieldInfoConsts.reserve(FieldInfos.size());
    for (unsigned I = 0, E = FieldInfos.size(); I != E; ++I)
    {
      const FieldInfoRecord &Info = FieldInfos[I];
      Constant *LabelStr = createStringLiteral(M, Info.EscapedLabel,
                                               "__field_profiler_struct." + Twine(I));
      Constant *FieldIdx = ConstantInt::get(Type::getInt32Ty(Ctx), Info.ID.FieldIndex);
      FieldInfoConsts.push_back(ConstantStruct::get(FieldInfoTy, {LabelStr, FieldIdx}));
    }
    auto *FieldInfoGV = new GlobalVariable(M, FieldInfoArrayTy, true,
                                           GlobalValue::InternalLinkage,
                                           ConstantArray::get(FieldInfoArrayTy, FieldInfoConsts),
                                           "__field_profiler_field_info");
    FieldInfoGV->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);
    FieldInfoGV->setAlignment(Align(8));

    std::vector<unsigned> Order(FieldInfos.size());
    std::iota(Order.begin(), Order.end(), 0);
    llvm::sort(Order, [&](unsigned L, unsigned R)
               {
                 const auto &A = FieldInfos[L];
                 const auto &B = FieldInfos[R];
                 if (A.StructLabel != B.StructLabel)
                   return A.StructLabel < B.StructLabel;
                 return A.ID.FieldIndex < B.ID.FieldIndex;
               });

    ArrayType *OrderArrayTy = ArrayType::get(Type::getInt32Ty(Ctx), Order.size());
    std::vector<Constant *> OrderConsts;
    OrderConsts.reserve(Order.size());
    for (unsigned Index : Order)
      OrderConsts.push_back(ConstantInt::get(Type::getInt32Ty(Ctx), Index));
    auto *OrderGV = new GlobalVariable(M, OrderArrayTy, true,
                                       GlobalValue::InternalLinkage,
                                       ConstantArray::get(OrderArrayTy, OrderConsts),
                                       "__field_profiler_order");
    OrderGV->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);

    SmallString<256> RootDir(CandidateAnalysisOutputDir);
    if (RootDir.empty())
      RootDir = "candidate-analysis-report";

    StringRef ModuleId = M.getName();
    std::string ModuleLabel = ModuleId.empty() ? "module" : ModuleId.str();
    SmallString<256> ModuleDir(RootDir);
    sys::path::append(ModuleDir, sanitizeForFilename(ModuleLabel));

    SmallString<256> ReportPath(ModuleDir);
    sys::path::append(ReportPath, "field-access-profiler.yaml");

    Constant *RootDirStr = createStringLiteral(M, RootDir, "__field_profiler_root_dir");
    Constant *ModuleDirStr = createStringLiteral(M, ModuleDir, "__field_profiler_module_dir");
    Constant *ReportPathStr = createStringLiteral(M, ReportPath, "__field_profiler_report_path");
    Constant *EscapedModuleLabelStr = createStringLiteral(M, yaml::escape(ModuleLabel),
                                                          "__field_profiler_module_label_escaped");

    buildRuntimeReporting(M, CountersTy, CountersGV,
                          FieldInfoArrayTy, FieldInfoGV,
                          OrderArrayTy, OrderGV,
                          EscapedModuleLabelStr,
                          RootDirStr, ModuleDirStr, ReportPathStr);

    return PreservedAnalyses::none();
  }

} // namespace candidate
