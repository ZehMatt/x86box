#include "asmjittranslate.h"

namespace x86box {

static std::map<x86box::MnemonicType, uint32_t> mnemonicTranslateion =
{
    { MnemonicType::I_SUB, asmjit::x86::Inst::kIdSub },
    { MnemonicType::I_ADD, asmjit::x86::Inst::kIdAdd },
    { MnemonicType::I_MOV, asmjit::x86::Inst::kIdMov },
    { MnemonicType::I_PUSH, asmjit::x86::Inst::kIdPush },
    { MnemonicType::I_LEA, asmjit::x86::Inst::kIdLea },
    { MnemonicType::I_CMP, asmjit::x86::Inst::kIdCmp },
    { MnemonicType::I_TEST, asmjit::x86::Inst::kIdTest },
    { MnemonicType::I_XOR, asmjit::x86::Inst::kIdXor },
    { MnemonicType::I_AND, asmjit::x86::Inst::kIdAnd },
    { MnemonicType::I_NOT, asmjit::x86::Inst::kIdNot },
    { MnemonicType::I_NEG, asmjit::x86::Inst::kIdNeg },
    { MnemonicType::I_SHL, asmjit::x86::Inst::kIdShl },
    { MnemonicType::I_SHR, asmjit::x86::Inst::kIdShr },
};

asmjit::Operand convertOperandImm(const Operand& op)
{
    switch (op.size)
    {
    case OperandSize::SIZE_8:
        return asmjit::Imm(op.imm.val.i8);
    case OperandSize::SIZE_16:
        return asmjit::Imm(op.imm.val.i16);
    case OperandSize::SIZE_32:
        return asmjit::Imm(op.imm.val.i32);
#ifdef _M_X64
    case OperandSize::SIZE_64:
        return asmjit::Imm(op.imm.val.i64);
#endif
    case OperandSize::SIZE_AUTO:
#ifdef _M_X64
        return asmjit::Imm(op.imm.val.i64);
#else 
        return asmjit::Imm(op.imm.val.i32);
#endif
    }
    // Unhandled.
    __debugbreak();
    return asmjit::Imm(-1);
}

asmjit::X86Reg convertReg(RegisterIndex regIdx, OperandSize size, OperandPosition pos /* = OperandPosition::LOW*/)
{
    uint32_t regType;

    uint32_t regId = getLocalRegisterId(regIdx);
    RegisterType type = getRegisterType(regIdx);

    if (type == RegisterType::GP)
    {
        switch (size)
        {
        case OperandSize::SIZE_8:
            if (pos == OperandPosition::HIGH)
                regType = asmjit::x86::Reg::kTypeGp8Hi;
            else
                regType = asmjit::x86::Reg::kTypeGp8Lo;
            break;
        case OperandSize::SIZE_16:
            regType = asmjit::x86::Reg::kTypeGp16;
            break;
        case OperandSize::SIZE_32:
            regType = asmjit::x86::Reg::kTypeGp32;
            break;
        case OperandSize::SIZE_64:
            regType = asmjit::x86::Reg::kTypeGp64;
            break;
        case OperandSize::SIZE_AUTO:
#ifdef _M_X64
            regType = asmjit::x86::Reg::kTypeGp64;
#else 
            regType = asmjit::x86::Reg::kTypeGp32;
#endif
        }
    }
    else if (type == RegisterType::ST)
    {
        regType = asmjit::x86::Reg::kTypeSt;
    }
    else if (type == RegisterType::SSE)
    {
        switch (size)
        {
        case OperandSize::SIZE_128:
            regType = asmjit::x86::Reg::kTypeXmm;
            break;
        case OperandSize::SIZE_256:
            regType = asmjit::x86::Reg::kTypeYmm;
            break;
        case OperandSize::SIZE_512:
            regType = asmjit::x86::Reg::kTypeZmm;
            break;
        }
    }

    asmjit::X86Reg res;
    res.setTypeAndId(regType, regId);

    return res;
}

asmjit::Operand convertOperandReg(const Operand& op)
{
    return convertReg(op.reg.reg, op.size, op.reg.pos);
}

asmjit::Operand convertOperandMem(const Operand& op)
{
    asmjit::X86Mem mem;
    if (op.mem.segment != SegmentReg::NONE)
    {
        switch (op.mem.segment)
        {
        case SegmentReg::CS:
            mem.setSegment(asmjit::x86::cs);
            break;
        case SegmentReg::DS:
            mem.setSegment(asmjit::x86::ds);
            break;
        case SegmentReg::GS:
            mem.setSegment(asmjit::x86::gs);
            break;
        case SegmentReg::FS:
            mem.setSegment(asmjit::x86::fs);
            break;
        }
    }
    if (op.mem.regBase != RegisterIndex::NONE)
    {
        mem.setBase(convertReg(op.mem.regBase, op.mem.addressSize));
    }
    if (op.mem.regIndex != RegisterIndex::NONE)
    {
        mem.setIndex(convertReg(op.mem.regIndex, op.mem.addressSize));
    }
    mem.setOffsetLo32(op.mem.disp.i32);

    return mem;
}

asmjit::Operand convertOperand(const Operand& op)
{
    switch (op.type)
    {
    case OperandType::IMM:
        return convertOperandImm(op);
    case OperandType::REG:
        return convertOperandReg(op);
    case OperandType::MEMORY:
        return convertOperandMem(op);
    }
    return asmjit::Operand();
}

uint32_t convertMnemonic(MnemonicType mnemonic)
{
    auto it = mnemonicTranslateion.find(mnemonic);
    if(it != mnemonicTranslateion.end())
        return it->second;

    return asmjit::x86::Inst::kIdNone;
}

}
