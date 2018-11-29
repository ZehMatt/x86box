#ifndef _X86BOX_INSTRUCTION_H_
#define _X86BOX_INSTRUCTION_H_
#pragma once

#include "operand.h"
#include "mnemonic.h"

#include <memory>

namespace x86box {

enum class Prefix : uint8_t
{
    NONE = 0,
    LOCK,
    REP,
    REPNE,
};

#pragma pack(push, 1)

struct Instruction
{
    Instruction(Prefix p, MnemonicType m, Operand ops[4])
        : prefix(p),
        mnemonic(m)
    {
        memcpy(operands, ops, sizeof(Operand) * 4);
    }
    Prefix prefix;
    MnemonicType mnemonic;
    Operand operands[4];
};

#pragma pack(pop)

}

#endif // _X86BOX_INSTRUCTION_H_