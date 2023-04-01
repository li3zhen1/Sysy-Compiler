//
// Created by 李臻 on 2022/5/22.
//

#pragma once

#include <iostream>
#include <string>

#include <iostream>

#include <unordered_map>
#include "bridge.h"
#include "func.h"
#include "asm.h"
#include "record.h"

#ifndef __APPLE__
#include <cassert>
#endif

using namespace std;

namespace risk {

    class Program {

        unordered_map<KValue, string> values = unordered_map<KValue, string>();
        unique_ptr<Function> currentFunction = nullptr;

#define VISIT(RAW) void visit(const RAW &raw, ostream& os);

    public:
        VISIT(KProgram)

    private:

        string visit(const KBasicBlock &, ostream &);

        Record visit(const KValue &, ostream &);

        VISIT(KFunction)

        VISIT(KValueData)

        VISIT(KReturn)

        VISIT(KInteger)

        VISIT(KType)

        VISIT(KGlobalAlloc)


#undef VISIT_GLOBAL
#undef VISIT


        bool isGlobal(const KValue &val) const;

        void setValue(const string &name, const KValue &val);

    };
}
