#include "x86box/translatorunit.h"
#include "jitcodegenerator.h"
#include "jitemulator.h"

namespace x86box {

TranslatorUnit::TranslatorUnit(IEmulator* emulator, uintptr_t vIP)
    : _parent(emulator),
    _virtualIP(vIP),
    _func(nullptr)
{
    _generator = std::make_unique<JitCodeGenerator>();
}

TranslatorUnit::~TranslatorUnit()
{
    reset();
}

bool TranslatorUnit::generate(ITranslator *translator)
{
    JitCodeGenerator *generator = reinterpret_cast<JitCodeGenerator*>(_generator.get());

    asmjit::JitRuntime *runtime = reinterpret_cast<asmjit::JitRuntime *>(_parent->getRuntime());

    asmjit::CodeHolder code;
    code.init(runtime->codeInfo());

    asmjit::x86::Builder builder(&code);

    if (!translator->process(generator))
    {
        return false;
    }

    if (!generator->generate(builder))
    {
        return false;
    }

    builder.finalize();

    runtime->add(&_func, &code);

    return true;
}

bool TranslatorUnit::isGenerated() const
{
    return _func != nullptr;
}

bool TranslatorUnit::execute(VContext& ctx, IMemoryHandler *memoryHandler)
{
    VContextInternal& _ctx = *reinterpret_cast<VContextInternal*>(&ctx);
    _ctx.memoryHandler = memoryHandler;

    if (!_func)
    {
        return false;
    }

    _func(ctx);

    return true;
}

void TranslatorUnit::reset()
{
    asmjit::JitRuntime *runtime = reinterpret_cast<asmjit::JitRuntime *>(_parent->getRuntime());

    if (_func)
    {
        runtime->release(_func);
        _func = nullptr;
    }
}

} // x86box
