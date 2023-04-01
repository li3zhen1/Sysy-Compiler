//
// Created by 李臻 on 2022/5/23.
//

#pragma once

#include <iostream>
#include <ostream>
#include <string>
#include <memory>

#include "func.h"
#include "bridge.h"
#include "record.h"

using namespace std;

namespace risk {

    class AsmStream {
        ostream &os;
        string holder;
    public:
        explicit AsmStream(ostream &_os, string _holder);

        explicit AsmStream(ostream &_os);

#define OP const string &

        AsmStream &li(OP dst, int imm);

        AsmStream &la(OP dst, OP sym);

        AsmStream &mv(OP dst, OP src);

        AsmStream &j(OP label);

        AsmStream &call(OP fun);

        AsmStream &sw(OP src, OP addr, int offset);

        AsmStream &lw(OP dst, OP addr, int offset);

        AsmStream &op(OP op, OP dst, OP lhs, OP rhs);

        AsmStream &op(OP op, OP dst, OP src);

        AsmStream &addi(OP dst, OP src, int imm);

        AsmStream &muli(OP dst, OP src, int imm);

        AsmStream &slli(OP dst, OP src, int imm);

        AsmStream &bnez(OP cond, OP label);

#undef OP
    };


}
