#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
class NullCheckPass : public MachineFunctionPass {
public:
  static char ID;
  NullCheckPass() : MachineFunctionPass(ID) {}

  bool runOnMachineFunction(MachineFunction &MF) override {
    const X86Subtarget &STI = MF.getSubtarget<X86Subtarget>();
    const X86InstrInfo *TII = STI.getInstrInfo();
    bool Changed = false;

    LLVMContext &Ctx = MF.getFunction().getContext();
    Function *AbortFunc = MF.getFunction().getParent()->getFunction("abort");
    if (!AbortFunc) {
      FunctionType *FTy = FunctionType::get(Type::getVoidTy(Ctx), false);
      AbortFunc = Function::Create(FTy, GlobalValue::ExternalLinkage, "abort",
                                   MF.getFunction().getParent());
    }

    for (auto &MBB : MF) {
      SmallVector<MachineInstr *, 8> MemInsts;
      for (auto &MI : MBB)
        if (MI.mayLoad() || MI.mayStore())
          MemInsts.push_back(&MI);

      for (MachineInstr *MI : MemInsts) {
        unsigned PtrReg = 0;
        for (const MachineOperand &MO : MI->operands()) {
          if (MO.isReg() && MO.isUse() && !MO.isDef()) {
            PtrReg = MO.getReg();
            break;
          }
        }
        if (!PtrReg)
          continue;

        MachineBasicBlock *CheckBB = &MBB;
        MachineBasicBlock *ContBB = MF.CreateMachineBasicBlock();
        MachineBasicBlock *TrapBB = MF.CreateMachineBasicBlock();

        MF.insert(std::next(CheckBB->getIterator()), ContBB);
        MF.insert(std::next(ContBB->getIterator()), TrapBB);

        ContBB->splice(ContBB->end(), CheckBB, std::next(MI->getIterator()),
                       CheckBB->end());
        ContBB->transferSuccessors(CheckBB);

        auto InsertPos = MI->getIterator();
        DebugLoc DL = MI->getDebugLoc();

        BuildMI(*CheckBB, InsertPos, DL, TII->get(X86::TEST64rr))
            .addReg(PtrReg)
            .addReg(PtrReg);
        BuildMI(*CheckBB, InsertPos, DL, TII->get(X86::JCC_1))
            .addImm(X86::COND_E)
            .addMBB(TrapBB);
        BuildMI(*CheckBB, InsertPos, DL, TII->get(X86::JMP_1)).addMBB(ContBB);

        BuildMI(*TrapBB, TrapBB->end(), DL, TII->get(X86::CALL64pcrel32))
            .addGlobalAddress(AbortFunc);
        BuildMI(*TrapBB, TrapBB->end(), DL, TII->get(X86::RET64));

        Changed = true;
      }
    }
    return Changed;
  }
};
char NullCheckPass::ID = 0;

static RegisterPass<NullCheckPass>
    X("null-check", "Insert null check before pointer dereference", false,
      false);
} // namespace