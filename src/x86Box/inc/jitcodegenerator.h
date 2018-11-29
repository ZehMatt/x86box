#ifndef _X86BOX_JITCODEGENERATOR_H_
#define _X86BOX_JITCODEGENERATOR_H_
#pragma once

#include "x86box/common.h"
#include "x86box/codegenerator.h"
#include "x86box/vcontext.h"
#include "x86box/memoryhandler.h"

#include "asmjit/asmjit.h"

#include <vector>
#include <unordered_set>

namespace x86box {

struct VContextInternal
{
    // Public.
    uint8_t _data[sizeof(VContext) - VContext::k_InternalSize];
    // Private.
    IMemoryHandler *memoryHandler;
};

class JitCodeGenerator : public ICodeGenerator
{
    struct RegHasher
    {
        size_t operator()(const asmjit::x86::Reg& reg) const
        {
            size_t h = ((size_t)reg.group() << 16) | reg.id();
            return h;
        }
    };

    struct GeneratorContext_t
    {
        uint32_t flagsIn = 0;
        uint32_t flagsOut = 0;
        std::unordered_set<asmjit::x86::Reg, RegHasher> regsRead;
        std::unordered_set<asmjit::x86::Reg, RegHasher> regsModified;
        asmjit::x86::Gp regContextBase;
        asmjit::FuncDetail funcDetail;
        asmjit::FuncFrame funcFrame;
    };

private:
    std::vector<Instruction> _scheduled;

public:
    JitCodeGenerator();

    virtual bool schedule(const Prefix& prefix, const MnemonicType mnemonic, Operand operands[4]) override;

    bool generate(asmjit::x86::Builder& builder);

private:
    bool generateInstruction(GeneratorContext_t& ctx, asmjit::x86::Builder& builder, const Instruction& instr);
    bool analyseContextUsage(GeneratorContext_t& ctx, asmjit::x86::Builder& builder, asmjit::CBNode *nodeStart, asmjit::CBNode *nodeEnd);
    bool generateContextEntry(GeneratorContext_t& ctx, asmjit::x86::Builder& builder, asmjit::CBNode *nodePos);
    bool generateContextExit(GeneratorContext_t& ctx, asmjit::x86::Builder& builder, asmjit::CBNode *nodePos);
};

}

#endif // _X86BOX_JITCODEGENERATOR_H_