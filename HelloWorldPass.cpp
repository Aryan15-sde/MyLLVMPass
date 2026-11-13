#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/BasicBlock.h"
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
          if (BinOp->getOpcode() == Instruction::Add) {
            IRBuilder<> Builder(BinOp);
            Value *NewSub = Builder.CreateSub(
              BinOp->getOperand(0), BinOp->getOperand(1), "subtmp");
            BinOp->replaceAllUsesWith(NewSub);
            ToErase.push_back(BinOp);
          }
        }
      }
    }

    for (Instruction *Inst : ToErase) {
      Inst->eraseFromParent();
    }

    if (!ToErase.empty()) {
      errs() << "Transformed " << ToErase.size()
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
