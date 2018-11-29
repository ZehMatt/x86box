#pragma once

#include <vector>
#include <unordered_map>
#include <map>

#include "x86box/mnemonic.h"
#include "x86box/operand.h"
#include "asmjit/asmjit.h"

namespace x86box {

    asmjit::Operand convertOperandImm(const Operand& op);
    asmjit::X86Reg convertReg(RegisterIndex regIdx, OperandSize size, OperandPosition pos = OperandPosition::LOW);
    asmjit::Operand convertOperandReg(const Operand& op);
    asmjit::Operand convertOperandMem(const Operand& op);
    asmjit::Operand convertOperand(const Operand& op);
    uint32_t convertMnemonic(MnemonicType mnemonic);

}
