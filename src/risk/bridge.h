//
// Created by 李臻 on 2022/5/23.
//

#pragma once

#include <koopa.h>
#include <string>

using namespace std;

namespace risk {

    bool isIMM12(int x);

    string dropFirst(const string &str);

    typedef koopa_raw_program_t KProgram;
    typedef koopa_raw_function_t KFunction;
    typedef koopa_raw_basic_block_t KBasicBlock;
    typedef koopa_raw_value_t KValue;
    typedef koopa_raw_value_data_t KValueData;
    typedef koopa_raw_type_t KType;
    typedef koopa_raw_slice_t KSlice;
    typedef koopa_raw_return_t KReturn;
    typedef koopa_raw_call_t KCall;
    typedef koopa_raw_integer_t KInteger;
    typedef koopa_raw_global_alloc_t KGlobalAlloc;
    typedef koopa_raw_jump_t KJump;
    typedef koopa_raw_binary_t KBinary;
    typedef koopa_raw_branch_t KBranch;
    typedef koopa_raw_value_data_t KValueData;

    unsigned int SizeOf(const KType &t);

    struct Slot {
        unsigned int offset;
        bool isPointer;
    };

    bool isGlobal(koopa_raw_value_tag_t tag);
    bool isLocal(koopa_raw_value_tag_t tag);

    enum class KBinaryOp {
        /// Not equal to.
        NotEq = KOOPA_RBO_NOT_EQ,
        /// Equal to.
        Eq = KOOPA_RBO_EQ,
        /// Greater than.
        Gt = KOOPA_RBO_GT,
        /// Less than.
        Lt = KOOPA_RBO_LT,
        /// Greater than or equal to.
        Ge = KOOPA_RBO_GE,
        /// Less than or equal to.
        Le = KOOPA_RBO_LE,
        /// Addition.
        Add = KOOPA_RBO_ADD,
        /// Subtraction.
        Sub = KOOPA_RBO_SUB,
        /// Multiplication.
        Mul = KOOPA_RBO_MUL,
        /// Division.
        Div = KOOPA_RBO_DIV,
        /// Modulo.
        Mod = KOOPA_RBO_MOD,
        /// Bitwise AND.
        And = KOOPA_RBO_AND,
        /// Bitwise OR.
        Or = KOOPA_RBO_OR,
        /// Bitwise XOR.
        Xor = KOOPA_RBO_XOR,
        /// Shift left logical.
        Shl = KOOPA_RBO_SHL,
        /// Shift right logical.
        Shr = KOOPA_RBO_SHR,
        /// Shift right arithmetic.
        Sar = KOOPA_RBO_SAR,
    };
}