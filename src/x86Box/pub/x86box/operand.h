#ifndef _X86BOX_OPERAND_H_
#define _X86BOX_OPERAND_H_
#pragma once

#include "common.h"

namespace x86box {

enum class OperandType : uint8_t
{
    NONE = 0,
    REG,
    IMM,
    MEMORY,
};

enum class OperandSize : uint8_t
{
    SIZE_8,
    SIZE_16,
    SIZE_32,
    SIZE_64,
    SIZE_128,
    SIZE_256,
    SIZE_512,
    SIZE_AUTO,
};

enum class SegmentReg : uint8_t
{
    NONE = 0,
    CS = 0,
    SS,
    DS,
    ES,
    FS,
    GS,
};

enum class RegisterIndex : uint8_t
{
    NONE = 0,
    // GP
    GP_REG0, // AL, AH, AX, EAX, RAX
    GP_REG1, // CL, CH, CX, ECX, RCX
    GP_REG2, // DL, DH, DX, EDX, RDX
    GP_REG3, // BL, BH, BX, EBX, RBX
    GP_REG4, // SPL, SP, ESP, RSP
    GP_REG5, // BPL, BP, EBP, RBP
    GP_REG6, // SIL, SI, ESI, RSI
    GP_REG7, // DIL, DI, EDI, RDI
    GP_REG8, // R8L, R8W, R8D, R8
    GP_REG9, // R9L, R9W, R9D, R9
    GP_REG10, // R10L, R10W, R10D, R10
    GP_REG11, // R11L, R11W, R11D, R11
    GP_REG12, // R12L, R12W, R12D, R12
    GP_REG13, // R13L, R13W, R13D, R13
    GP_REG14, // R14L, R14W, R14D, R14
    GP_REG15, // R15L, R15W, R15D, R15
    // FPU (ST, MM)
    ST_REG0,
    ST_REG1,
    ST_REG2,
    ST_REG3,
    ST_REG4,
    ST_REG5,
    ST_REG6,
    ST_REG7,
    ST_REG8,
    ST_REG9,
    ST_REG10,
    ST_REG11,
    ST_REG12,
    ST_REG13,
    ST_REG14,
    ST_REG15,
    // SSE (XMM, YMM, ZMM)
    SSE_REG0,
    SSE_REG1,
    SSE_REG2,
    SSE_REG3,
    SSE_REG4,
    SSE_REG5,
    SSE_REG6,
    SSE_REG7,
    SSE_REG8,
    SSE_REG9,
    SSE_REG10,
    SSE_REG11,
    SSE_REG12,
    SSE_REG13,
    SSE_REG14,
    SSE_REG15,
    SSE_REG16,
    SSE_REG17,
    SSE_REG18,
    SSE_REG19,
    SSE_REG20,
    SSE_REG21,
    SSE_REG22,
    SSE_REG23,
    SSE_REG24,
    SSE_REG25,
    SSE_REG26,
    SSE_REG27,
    SSE_REG28,
    SSE_REG29,
    SSE_REG30,
    SSE_REG31,
};

enum class RegisterType : uint8_t
{
    GP,
    ST,
    SSE,
};

enum class OperandPosition : uint8_t
{
    LOW = 0,
    HIGH,
};

enum class MemoryScale : uint8_t
{
    SCALE_1 = 0,
    SCALE_2,
    SCALE_4,
    SCALE_8,
    SCALE_16,
};

static uint32_t getLocalRegisterId(RegisterIndex idx)
{
    uint32_t groupStart = 0;

    if (idx >= RegisterIndex::GP_REG0 && idx <= RegisterIndex::GP_REG15)
        groupStart = (uint32_t)RegisterIndex::GP_REG0;
    else if (idx >= RegisterIndex::ST_REG0 && idx <= RegisterIndex::ST_REG15)
        groupStart = (uint32_t)RegisterIndex::ST_REG0;
    else if (idx >= RegisterIndex::SSE_REG0 && idx <= RegisterIndex::SSE_REG31)
        groupStart = (uint32_t)RegisterIndex::SSE_REG0;

    return (uint32_t)idx - groupStart;
}

static RegisterType getRegisterType(RegisterIndex idx)
{
    if (idx >= RegisterIndex::GP_REG0 && idx <= RegisterIndex::GP_REG15)
        return RegisterType::GP;
    else if (idx >= RegisterIndex::ST_REG0 && idx <= RegisterIndex::ST_REG15)
        return RegisterType::ST;
    else if (idx >= RegisterIndex::SSE_REG0 && idx <= RegisterIndex::SSE_REG31)
        return RegisterType::SSE;

    return RegisterType::GP;
}

#pragma pack(push, 1)

struct OperandRegister
{
    enum { k_OperandType = OperandType::REG };

    RegisterIndex reg;
    OperandPosition pos;

    bool isGPReg() const
    {
        return getRegisterType(reg) == RegisterType::GP;
    }

    bool isSTReg() const
    {
        return getRegisterType(reg) == RegisterType::ST;
    }

    bool isSSEReg() const
    {
        return getRegisterType(reg) == RegisterType::SSE;
    }

    uint32_t localId() const
    {
        return getLocalRegisterId(reg);
    }
};

struct OperandMemory
{
    enum { k_OperandType = OperandType::MEMORY };

    OperandSize addressSize;
    SegmentReg segment : 4;
    MemoryScale scale : 4;
    RegisterIndex regBase : 4;
    RegisterIndex regIndex : 4;
    Integer32 disp;
};

struct OperandImm
{
    enum { k_OperandType = OperandType::IMM };

    Integer val;
};

struct Operand
{
    OperandType type : 4;
    OperandSize size : 4;
    union
    {
        OperandImm imm;
        OperandRegister reg;
        OperandMemory mem;
    };
};

#pragma pack(pop)

// template<int> struct ReportSize;
// ReportSize < sizeof(Operand) > Foo;

}

#endif // _X86BOX_OPERAND_H_