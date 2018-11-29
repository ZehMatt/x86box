#ifndef _X86BOX_JITEMULATOR_H_
#define _X86BOX_JITEMULATOR_H_
#pragma once

#include "x86box/common.h"
#include "x86box/emulator.h"
#include "x86box/translatorunit.h"

#include "asmjit/asmjit.h"

#include <unordered_map>

namespace x86box {

class JitEmulator : public IEmulator
{
private:
    asmjit::JitRuntime _runtime;
    std::unordered_map<uintptr_t, std::unique_ptr<TranslatorUnit>> _units;

public:
    JitEmulator();
    virtual ~JitEmulator();

    virtual void* getRuntime() const override
    {
        return (void*)&_runtime;
    }

    virtual TranslatorUnit* findUnit(uintptr_t vIP) override;
    virtual TranslatorUnit* createUnit(uintptr_t vIP) override;

    virtual void releaseUnit(TranslatorUnit *unit) override;
    virtual void releaseAllUnits() override;
};

}

#endif // _X86BOX_JITEMULATOR_H_