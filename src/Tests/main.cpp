#include "x86box/x86box.h"
#include <windows.h>

using namespace x86box;

class BasicMemoryHandler : public x86box::IMemoryHandler
{
public:
    virtual bool read(const void *src, void *target, size_t size)
    {
        switch (size)
        {
        case 1:
            *(uint8_t*)target = *(uint8_t*)src;
            return true;
        case 2:
            *(uint16_t*)target = *(uint16_t*)src;
            return true;
        case 4:
            *(uint32_t*)target = *(uint32_t*)src;
            return true;
        case 8:
            *(uint64_t*)target = *(uint64_t*)src;
            return true;
        }
        return false;
    }

    virtual bool write(const void *src, void *target, size_t size)
    {
        switch (size)
        {
        case 1:
            *(uint8_t*)target = *(uint8_t*)src;
            return true;
        case 2:
            *(uint16_t*)target = *(uint16_t*)src;
            return true;
        case 4:
            *(uint32_t*)target = *(uint32_t*)src;
            return true;
        case 8:
            *(uint64_t*)target = *(uint64_t*)src;
            return true;
        }
        return false;
    }
};

class BasicTranslation : public x86box::ITranslator
{
public:
    virtual bool process(x86box::ICodeGenerator *gen) override
    {
        // sub zax, zcx
        {
            Operand ops[4] = {};

            ops[0].type = OperandType::REG;
            ops[0].size = OperandSize::SIZE_32;
            ops[0].reg.reg = RegisterIndex::GP_REG0; // zax

            ops[1].type = OperandType::REG;
            ops[1].size = OperandSize::SIZE_32;
            ops[1].reg.reg = RegisterIndex::GP_REG1; // zcx

            gen->schedule(Prefix::NONE, MnemonicType::I_SUB, ops);
        }

        // mov zbx, ptr [zdx] ; Read
        {
            Operand ops[4] = {};

            ops[0].type = OperandType::REG;
            ops[0].size = OperandSize::SIZE_32;
            ops[0].reg.reg = RegisterIndex::GP_REG3; // zbx

            ops[1].type = OperandType::MEMORY;
            ops[1].size = OperandSize::SIZE_32;
            ops[1].mem.addressSize = OperandSize::SIZE_AUTO;
            ops[1].mem.regBase = RegisterIndex::GP_REG2; // zdx

            gen->schedule(Prefix::NONE, MnemonicType::I_MOV, ops);
        }

        // mov ptr [zdx], zax ; Write
        {
            Operand ops[4] = {};

            ops[0].type = OperandType::MEMORY;
            ops[0].size = OperandSize::SIZE_32;
            ops[0].mem.addressSize = OperandSize::SIZE_AUTO;
            ops[0].mem.regBase = RegisterIndex::GP_REG2; // zdx

            ops[1].type = OperandType::REG;
            ops[1].size = OperandSize::SIZE_32;
            ops[1].reg.reg = RegisterIndex::GP_REG0; // zax

            gen->schedule(Prefix::NONE, MnemonicType::I_MOV, ops);
        }

        return true;
    }
};

template<typename A, typename B>
void assertEq(const A a, const B b)
{
    if(a != b)
        __debugbreak();
}

int main()
{
    IEmulator *emulator = x86box::createEmulator();

    BasicTranslation translation;
    BasicMemoryHandler memoryHandler;

    // Create unit and generate code via translation.
    {
        TranslatorUnit *unit1 = emulator->createUnit(0x00400000);

        if (!unit1->generate(&translation))
        {
            printf("Unable to generate code.\n");
            return -1;
        }

        if (!unit1->isGenerated())
        {
            printf("Unit is empty.\n");
            return -1;
        }
    }

    uint64_t tickStart = GetTickCount64();
    uint64_t tickLast = tickStart;
    uint64_t numExec = 0;

    // Virtual Context.
    VContext ctx = {};

    TranslatorUnit *unit1 = emulator->findUnit(0x00400000);

    while (true)
    {
        uint64_t curTick = GetTickCount64();

        ctx.gpRegs[0].val.u32 = 0xBADBABE; // zax;
        ctx.gpRegs[1].val.u32 = 0xDEADBEEF; // zcx
        uint32_t expected1 = ctx.gpRegs[0].val.u32 - ctx.gpRegs[1].val.u32;
        uint32_t expected2 = 0xF00BA7C2;

        uint32_t memoryValue = expected2;
        ctx.gpRegs[2].val.ptr = &memoryValue;

        // Find and execute a specific unit.
        {
            if (!unit1->execute(ctx, &memoryHandler))
            {
                printf("Unable to execute code.\n");
                return -1;
            }

            assertEq(ctx.gpRegs[0].val.u32, expected1);
            assertEq(ctx.gpRegs[3].val.u32, expected2);
            assertEq(memoryValue, expected1);
        }

        numExec++;
        if (curTick - tickLast >= 1000)
        {
            double perSec = numExec / ((curTick - tickStart) / 1000.0);
            tickLast = curTick;
            printf("\rExec: %0.5f", perSec);
        }
    }

    emulator->releaseUnit(unit1);

    return 0;
}

