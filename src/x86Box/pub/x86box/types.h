#ifndef _X86BOX_TYPES_H_
#define _X86BOX_TYPES_H_
#pragma once

#include <stdint.h>

namespace x86box {

#pragma pack(push, 1)

typedef union
{
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
#ifdef _M_AMD64 
    uint64_t u64;
#endif
    int8_t i8;
    int16_t i16;
    int32_t i32;
#ifdef _M_AMD64 
    int64_t i64;
#endif
    void *ptr;
} Integer;

typedef union
{
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    int8_t i8;
    int16_t i16;
    int32_t i32;
} Integer32;

#pragma pack(pop)

}

#endif // _X86BOX_TYPES_H_