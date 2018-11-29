#ifndef _X86BOX_EMULATOR_H_
#define _X86BOX_EMULATOR_H_
#pragma once

#include "common.h"
#include "vcontext.h"
#include "translator.h"
#include "translatorunit.h"
#include "memoryhandler.h"

namespace x86box {

class TranslatorUnit;

class IEmulator
{
public:
    virtual void* getRuntime() const = 0;

    virtual TranslatorUnit* findUnit(uintptr_t vIP) = 0;
    virtual TranslatorUnit* createUnit(uintptr_t vIP) = 0;
    virtual void releaseUnit(TranslatorUnit *unit) = 0;
    virtual void releaseAllUnits() = 0;
};

} // x86box.

#endif // _X86BOX_EMULATOR_H_
