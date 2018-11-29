#ifndef _X86BOX_CONTEXT_H_
#define _X86BOX_CONTEXT_H_
#pragma once

#include "common.h"
#include "gpreg.h"

namespace x86box {

#pragma pack(push, 1)
struct VContext
{
    enum { k_InternalSize = 0x100 };

    uintptr_t flags;
#ifdef _AMD64_
    GPReg gpRegs[16];
#else 
    GPReg gpRegs[8];
#endif

    uint8_t _reserved[k_InternalSize];
};
#pragma pack(pop)

} // x86box

#endif // _X86BOX_CONTEXT_H_