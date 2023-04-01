//
// Created by 李臻 on 2022/5/17.
//

#pragma once

#include <string>
#include <ostream>
#include <optional>
#include <vector>

#include "operand.h"
#include "type.h"

using namespace std;

namespace ir {
    enum class IrKind {
        Alloc,
        Br,
        Jump,
        Load,
        Store,
        Ret,

        GetElemPtr,
        GetPtr,

        Call,

        /// Not equal to.
        NotEq,
        /// Equal to.
        Eq,
        /// Greater than.
        Gt,
        /// Less than.
        Lt,
        /// Greater than or equal to.
        Ge,
        /// Less than or equal to.
        Le,
        /// Addition.
        Add,
        /// Subtraction.
        Sub,
        /// Multiplication.
        Mul,
        /// Division.
        Div,
        /// Modulo.
        Mod,
        /// Bitwise AND.
        And,
        /// Bitwise OR.
        Or,
        /// Bitwise XOR.
        Xor,
        /// Shift left logical.
        Shl,
        /// Shift right logical.
        Shr,
        /// Shift right arithmetic.
        Sar,
    };

    struct Instruction {
        IrKind kind;
        Operand op1;
        Operand op2;
        Operand op3;

        optional<string> callRet;
        vector<Operand> callParams;
        optional<Operand> ret;
        unique_ptr<Type> allocType;
        string callFun;

        Instruction(const IrKind &_kind);

        Instruction(const IrKind &_kind, const Operand &_op1);

        Instruction(const IrKind &_kind, const Operand &_op1, const Operand &_op2);

        Instruction(const IrKind &_kind, const Operand &_op1, const Operand &_op2, const Operand &_op3);

        Instruction(const string &_fun, const string &_callRet, const vector<Operand> &_callParams);

        Instruction(const string &_fun, const vector<Operand> &_callParams);

        Instruction(const string &_symbolName, const ir::Type &_ty);


        friend ostream &operator<<(ostream &os, const Instruction &inst);
    };
}