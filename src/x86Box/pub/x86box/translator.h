#ifndef _X86BOX_TRANSLATOR_H_
#define _X86BOX_TRANSLATOR_H_
#pragma once

#include "codegenerator.h"

namespace x86box {

class ITranslator
{
public:
    virtual bool process(ICodeGenerator *gen) = 0;
};

}

#endif // _X86BOX_TRANSLATOR_H_