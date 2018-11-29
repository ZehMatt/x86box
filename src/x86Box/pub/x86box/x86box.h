#ifndef _X86BOX_H_
#define _X86BOX_H_
#pragma once

#include "common.h"
#include "emulator.h"

namespace x86box {

IEmulator* __cdecl createEmulator();

}

#endif // _X86BOX_H_