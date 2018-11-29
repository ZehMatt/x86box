#include "x86box/x86box.h"
#include "jitemulator.h"

namespace x86box {

IEmulator* x86box::createEmulator()
{
    return new JitEmulator();
}

}