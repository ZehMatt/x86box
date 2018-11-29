#ifndef _X86BOX_MEMORYHANDLER_H_
#define _X86BOX_MEMORYHANDLER_H_
#pragma once

#include "common.h"

namespace x86box {

class IMemoryHandler
{
public:
    virtual bool read(const void *src, void *target, size_t size) = 0;
    virtual bool write(const void *src, void *target, size_t size) = 0;
};

}

#endif // _X86BOX_MEMORYHANDLER_H_