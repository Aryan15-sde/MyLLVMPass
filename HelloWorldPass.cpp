#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {

struct HelloWorldPass : PassInfoMixin<HelloWorldPass> {
  static bool isRequired() { return true; }

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
    SmallVector<Instruction *, 8> ToErase;

    for (BasicBlock &BB : F) {
      for (Instruction &I : BB) {
        if (auto *BinOp = dyn_cast<BinaryOperator>(&I)) {
          if (BinOp->getOpcode() == Instruction::Add &&
              BinOp->getType()->isIntOrIntVectorTy()) {
            IRBuilder<> Builder(BinOp);
            Value *A = BinOp->getOperand(0);
            Value *B = BinOp->getOperand(1);

            Value *XorVal = Builder.CreateXor(A, B, "mba_xor");
            Value *AndVal = Builder.CreateAnd(A, B, "mba_and");
            Value *One = ConstantInt::get(A->getType(), 1);
            Value *ShlVal = Builder.CreateShl(AndVal, One, "mba_shl");
            Value *ObfuscatedAdd = Builder.CreateAdd(XorVal, ShlVal, "mba_add");

            BinOp->replaceAllUsesWith(ObfuscatedAdd);
            ToErase.push_back(BinOp);
          }
        }
      }
    }

    for (Instruction *Inst : ToErase) {
      Inst->eraseFromParent();
    }

    if (!ToErase.empty()) {
      errs() << "Obfuscated " << ToErase.size()
             << " add instruction(s) in function: " << F.getName() << "\n";
      return PreservedAnalyses::none();
    }

    return PreservedAnalyses::all();
  }
};

}

extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {
    LLVM_PLUGIN_API_VERSION, "HelloWorldPass", LLVM_VERSION_STRING,
    [](PassBuilder &PB) {
      PB.registerPipelineParsingCallback(
        [](StringRef Name, FunctionPassManager &FPM,
           ArrayRef<PassBuilder::PipelineElement>) {
          if (Name == "hello-world") {
            FPM.addPass(HelloWorldPass());
            return true;
          }
          return false;
        });
    }
  };
}
