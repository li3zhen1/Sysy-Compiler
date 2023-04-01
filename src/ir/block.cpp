//
// Created by 李臻 on 2022/5/9.
//

#include "block.h"


using namespace ir;

BasicBlock::BasicBlock(const string &_ident) : ident(_ident) {
    inst = vector<Instruction>();
}

void BasicBlock::pushInstruction(const IrKind &kind, const Operand &op1, const Operand &op2) {
    assert(!escape);
    inst.emplace_back(kind, op1, op2);
}

void BasicBlock::pushInstruction(const IrKind &kind, const Operand &op1, const Operand &op2, const Operand &op3) {
    assert(!escape);
    inst.emplace_back(kind, op1, op2, op3);
}


void BasicBlock::commitBlockByJump(const string &dst) {
    assert(!escape);
    escape = make_unique<Instruction>(
        IrKind::Jump, dst
    );
}

void BasicBlock::commitBlockByBranch(const Operand &cond, const string &branch1, const string &branch2) {
    assert(!escape);
    escape = make_unique<Instruction>(
        IrKind::Br, cond, branch1, branch2
    );
}

void BasicBlock::commitBlockByRet(const Operand &ret) {
    assert(!escape);
    escape = make_unique<Instruction>(
        IrKind::Ret, ret
    );
}

void BasicBlock::commitBlockByRet() {
    assert(!escape);
    escape = make_unique<Instruction>(
        IrKind::Ret
    );
}

ostream &ir::operator<<(ostream &os, const BasicBlock &block) {
    assert(block.escape);
    os << block.ident << ":\n";
    for (const auto &i: block.inst) {
        os << i;
    }
    os << *block.escape;
    return os;
}

void BasicBlock::pushCallInstruction(const string &_fun, const string &_callRet, const vector<Operand> &_callParams) {
    assert(!escape);
    inst.emplace_back(_fun, _callRet, _callParams);
}


void BasicBlock::pushCallInstruction(const string &_fun, const vector<Operand> &_callParams) {
    assert(!escape);
    inst.emplace_back(_fun, _callParams);
}
