#ifndef _X86BOX_TRANSLATORUNIT_H_
#define _X86BOX_TRANSLATORUNIT_H_

#pragma once

#include "x86box/common.h"
#include "x86box/codegenerator.h"
#include "x86box/emulator.h"
#include "x86box/memoryhandler.h"

namespace x86box {

class IEmulator;

class TranslatorUnit
{
    typedef void(*fnJitFunction)(VContext& ctx);

private:
    IEmulator * _parent;
    uintptr_t _virtualIP;
    std::unique_ptr<ICodeGenerator> _generator;
    fnJitFunction _func;

public:
    TranslatorUnit(IEmulator* emulator, uintptr_t vIP);
    virtual ~TranslatorUnit();

    virtual uintptr_t getVirtualIP() const
    {
        return _virtualIP;
    }

    virtual bool generate(ITranslator *translator);

    virtual bool isGenerated() const;

    virtual bool execute(VContext& ctx, IMemoryHandler *memoryHandler);

    virtual void reset();
};

}

#endif // _X86BOX_TRANSLATORUNIT_H_
