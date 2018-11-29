#ifndef _X86BOX_CODEGENERATOR_H_
#define _X86BOX_CODEGENERATOR_H_
#pragma once

#include "common.h"
#include "instruction.h"

namespace x86box {

class ICodeGenerator
{
public:
    virtual bool schedule(const Prefix& prefix, const MnemonicType mnemonic, Operand operands[4]) = 0;
};

}

#endif // _X86BOX_CODEGENERATOR_H_