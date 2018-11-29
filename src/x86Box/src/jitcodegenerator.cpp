#include "jitcodegenerator.h"
#include "asmjittranslate.h"

namespace x86box {

JitCodeGenerator::JitCodeGenerator()
{
}

bool JitCodeGenerator::schedule(const Prefix& prefix, const MnemonicType mnemonic, Operand operands[4])
{
    _scheduled.emplace_back(prefix, mnemonic, operands);

    return true;
}

bool JitCodeGenerator::generate(asmjit::x86::Builder& builder)
{
    GeneratorContext_t ctx;

    for (const Instruction& instr : _scheduled)
    {
        if (!generateInstruction(ctx, builder, instr))
        {
            return false;
        }
    }
    _scheduled.clear();

    asmjit::CBNode *nodePreGenerated = builder.firstNode();
    asmjit::CBNode *nodePostGenerated = builder.lastNode();

    ctx.funcDetail.init(asmjit::FuncSignatureT<void, void*>(asmjit::CallConv::kIdHost));
    ctx.funcFrame.init(ctx.funcDetail);

    if (!analyseContextUsage(ctx, builder, nodePreGenerated, nodePostGenerated))
    {
        return false;
    }

    // Insert before first instruction.
    if (!generateContextEntry(ctx, builder, nullptr))
    {
        return false;
    }

    // Insert after last.
    if (!generateContextExit(ctx, builder, nodePostGenerated))
    {
        return false;
    }

    return true;
}


int32_t getGPRegisterOffset(const asmjit::x86::Reg& reg)
{
    if (reg.isGp())
    {
        uint32_t idx = reg.id();
        return (int32_t)offsetof(VContext, gpRegs[idx]);
    }
    else
    {
        // TODO: Add other reg types.
    }
    return 0;
}

int32_t getEFlagsOffset()
{
    return offsetof(VContext, flags);
}

bool JitCodeGenerator::generateContextEntry(GeneratorContext_t& ctx, asmjit::x86::Builder& builder, asmjit::CBNode *nodePos)
{
    const auto& regBase = ctx.regContextBase;

    builder.setCursor(nodePos);

    asmjit::FuncArgsAssignment args(&ctx.funcDetail);
    args.assignAll(regBase);

    builder.emitProlog(ctx.funcFrame);
    builder.emitArgsAssignment(ctx.funcFrame, args);

    uint32_t gpSize = builder.gpSize();

    // Set flags if input is required.
    // NOTE: We spill eax/rax but its okay since registers are set after.
    if (ctx.flagsIn != 0)
    {
        int32_t flagsOffset = offsetof(VContext, flags);

        builder.push(asmjit::x86::ptr(regBase, flagsOffset));
        builder.popfd();
    }

    // Write all input registers.
    for (auto& regIn : ctx.regsRead)
    {
        int32_t regOffset = getGPRegisterOffset(regIn);

        if (regIn.isGp())
        {
            const asmjit::X86Gp& reg = builder.gpz(regIn.id());
            builder.mov(reg, asmjit::X86Mem(regBase, regOffset, gpSize));
        }
        else
        {
            // TODO: Add other reg types.
        }
    }

    return true;
}

bool JitCodeGenerator::generateContextExit(GeneratorContext_t& ctx, asmjit::x86::Builder& builder, asmjit::CBNode *nodePos)
{
    const auto& zax = builder.zax();
    const auto& zcx = builder.zcx();
    const auto& regBase = ctx.regContextBase;

    auto regTemp = zax;
    if (regBase == regTemp)
    {
        regTemp = zcx;
    }

    builder.setCursor(nodePos);

    uint32_t gpSize = builder.gpSize();

    if (ctx.flagsOut != 0)
    {
        // Store the flags so we can spill any we want.
        builder.pushfd();
    }

    // Write all output registers.
    for (auto& regIn : ctx.regsRead)
    {
        int32_t regOffset = getGPRegisterOffset(regIn);

        if (regIn.isGp())
        {
            const asmjit::X86Gp& reg = builder.gpz(regIn.id());
            builder.mov(asmjit::X86Mem(regBase, regOffset, gpSize), reg);
        }
        else
        {
            // TODO: Add other reg types.
        }
    }

    // Set flags in vctx.
    if (ctx.flagsOut != 0)
    {
        int32_t flagsOffset = offsetof(VContext, flags);

        builder.pop(regTemp);
        builder.mov(asmjit::x86::dword_ptr(regBase, flagsOffset), regTemp.r32());
    }

    builder.emitEpilog(ctx.funcFrame);

    return true;
}

bool JitCodeGenerator::generateInstruction(GeneratorContext_t& ctx, asmjit::x86::Builder& builder, const Instruction& instr)
{
    uint32_t instrId = convertMnemonic(instr.mnemonic);

    const asmjit::Operand op0 = convertOperand(instr.operands[0]);
    const asmjit::Operand op1 = convertOperand(instr.operands[1]);
    const asmjit::Operand op2 = convertOperand(instr.operands[2]);
    const asmjit::Operand op3 = convertOperand(instr.operands[3]);

    if (instr.prefix == Prefix::LOCK)
        builder.lock();
    else if (instr.prefix == Prefix::REP)
        builder.rep();
    else if (instr.prefix == Prefix::REPNE)
        builder.repne();

    builder.emit(instrId, op0, op1, op2, op3);

    return true;
}

bool JitCodeGenerator::analyseContextUsage(GeneratorContext_t& ctx, asmjit::x86::Builder& builder, asmjit::CBNode *nodeStart, asmjit::CBNode *nodeEnd)
{
    asmjit::CBNode *node = nodeStart;

    while (node != nullptr)
    {
        if (node->isInst())
        {
            asmjit::CBInst *inst = node->as<asmjit::CBInst>();
            asmjit::Operand *ops = inst->operands();

            const auto& instrData = asmjit::x86::InstDB::infoById(inst->id());
            ctx.flagsIn |= instrData.executionInfo().specialRegsR();
            ctx.flagsOut |= instrData.executionInfo().specialRegsW();

            for (size_t n = 0; n < inst->opCount(); n++)
            {
                asmjit::Operand& op = ops[n];
                if (op.isReg())
                {
                    asmjit::x86::Reg& reg = static_cast<asmjit::x86::Reg&>(op);

                    uint32_t regMask = 1u << reg.id();
                    //if (!(ctx.funcFrame.dirtyRegs(reg.group()) & regId))
                    {
                        // FIXME: This is currently not correct, look if asmjit has infos about operand access.
                        ctx.regsRead.insert(reg.clone());
                        ctx.regsModified.insert(reg.clone());

                        ctx.funcFrame.addDirtyReg(reg);
                    }
                }
                else if (op.isMem())
                {
                    asmjit::x86::Mem& mem = static_cast<asmjit::x86::Mem&>(op);

                    if (mem.hasBase())
                    {
                        uint32_t regId = mem.baseId();
                        uint32_t regMask = 1u << regId;
                        //if (!(ctx.funcFrame.dirtyRegs(asmjit::x86::Reg::kGroupGp) & regMask))
                        {
                            ctx.regsRead.insert(builder.gpz(regId));
                            ctx.funcFrame.addDirtyRegs(asmjit::x86::Reg::kGroupGp, regMask);
                        }
                    }
                    if (mem.hasIndex())
                    {
                        uint32_t regId = mem.indexId();
                        uint32_t regMask = 1u << regId;
                        //if (!(ctx.funcFrame.dirtyRegs(asmjit::x86::Reg::kGroupGp) & regMask))
                        {
                            ctx.regsRead.insert(builder.gpz(regId));
                            ctx.funcFrame.addDirtyRegs(asmjit::x86::Reg::kGroupGp, regMask);
                        }
                    }
                }
            }
        }
        node = node->next();
    }

    const auto& rsp = asmjit::x86::rsp;
    const auto& rbp = asmjit::x86::rbp;

    uint32_t dirtyRegs = ctx.funcFrame.dirtyRegs(asmjit::x86::Reg::kGroupGp);

    uint32_t count = builder.gpCount();
    for (uint32_t i = 0; i < count; i++)
    {
        if (i == rsp.id() || i == rbp.id())
        {
            // Can not touch stack.
            continue;
        }

        auto reg = builder.gpz(i);
        auto mask = 1u << i;

        if ((dirtyRegs & mask) == 0)
        {
            // Found ourselves a free register to use as base.
            ctx.regContextBase = reg;

            // Mark also as dirty.
            ctx.funcFrame.addDirtyRegs(reg.group(), mask);
            break;
        }
    }

    ctx.funcFrame.finalize();

    return true;
}

}
