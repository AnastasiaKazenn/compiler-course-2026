#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

using namespace llvm;

namespace {

class KazennovaFMulAddPass : public PassInfoMixin<KazennovaFMulAddPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM) {
    bool Changed = false;

    for (BasicBlock &BB : F) {
      for (Instruction &I : llvm::make_early_inc_range(BB)) {
        auto *Call = dyn_cast<CallInst>(&I);
        if (!Call)
          continue;

        Function *Callee = Call->getCalledFunction();
        if (!Callee)
          continue;

        if (Callee->getIntrinsicID() == Intrinsic::fmuladd) {
          replaceFMulAdd(Call);
          Changed = true;
        }
      }
    }

    return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
  }

private:
  void replaceFMulAdd(CallInst *Call) {
    if (Call->use_empty()) {
      Call->eraseFromParent();
      return;
    }

    IRBuilder<> Builder(Call);

    Value *a = Call->getOperand(0);
    Value *b = Call->getOperand(1);
    Value *c = Call->getOperand(2);

    Value *mul = Builder.CreateFMul(a, b);
    Value *add = Builder.CreateFAdd(mul, c);

    if (auto *FMul = dyn_cast<Instruction>(mul))
      FMul->copyFastMathFlags(Call);
    if (auto *FAdd = dyn_cast<Instruction>(add))
      FAdd->copyFastMathFlags(Call);

    if (auto *MulInst = dyn_cast<Instruction>(mul)) {
      MulInst->setDebugLoc(Call->getDebugLoc());
      MulInst->copyMetadata(*Call);
    }
    if (auto *AddInst = dyn_cast<Instruction>(add)) {
      AddInst->setDebugLoc(Call->getDebugLoc());
      AddInst->copyMetadata(*Call);
    }

    Call->replaceAllUsesWith(add);
    Call->eraseFromParent();
  }
};

} // namespace

extern "C" ::llvm::PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "KazennovaFMulAddPass", "v0.1",
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "decompose-fmuladd") {
                    FPM.addPass(KazennovaFMulAddPass());
                    return true;
                  }
                  return false;
                });
          }};
}