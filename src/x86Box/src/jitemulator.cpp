#include "jitemulator.h"
#include "jitcodegenerator.h"

namespace x86box {

JitEmulator::JitEmulator()
{
}

JitEmulator::~JitEmulator()
{
}

TranslatorUnit* JitEmulator::findUnit(uintptr_t vIP)
{
    auto itr = _units.find(vIP);
    if (itr != _units.end())
    {
        return itr->second.get();
    }
    return nullptr;
}

TranslatorUnit* JitEmulator::createUnit(uintptr_t vIP)
{
    TranslatorUnit* unit = findUnit(vIP);
    if(unit)
        return unit;

    auto it = _units.emplace(vIP, std::make_unique<TranslatorUnit>(this, vIP));
    return (*it.first).second.get();
}

void JitEmulator::releaseUnit(TranslatorUnit *unit)
{
    if(!unit)
        return;

    _units.erase(unit->getVirtualIP());
}

void JitEmulator::releaseAllUnits()
{
    _units.clear();
}

} // x86box
