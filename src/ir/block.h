//
// Created by 李臻 on 2022/5/9.
//

#pragma once

#include <memory>

#ifndef __APPLE__
#include <cassert>
#endif

#include "ast.h"
#include "inst.h"
#include "operand.h"
#include "value.h"

using namespace std;

namespace ir {
    class BasicBlock {
    public:
        string ident;

        BasicBlock(const string &_ident);

        vector<Instruction> inst;
        unique_ptr<Instruction> escape;

        bool isolated = false;

        void pushInstruction(const IrKind &kind, const Operand &op1, const Operand &op2);

        void pushInstruction(const IrKind &kind, const Operand &op1, const Operand &op2, const Operand &op3);

        void pushCallInstruction(const string &_fun, const string &_callRet, const vector<Operand> &_callParams);

        void pushCallInstruction(const string &_fun, const vector<Operand> &_callParams);

        void commitBlockByJump(const string &);

        void commitBlockByBranch(const Operand &cond, const string &branch1, const string &branch2);

        void commitBlockByRet(const Operand &ret);

        void commitBlockByRet();

        friend ostream &operator<<(ostream &os, const BasicBlock &block);


    };
}