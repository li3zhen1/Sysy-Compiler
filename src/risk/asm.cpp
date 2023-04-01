//
// Created by 李臻 on 2022/5/23.
//

#include "asm.h"

using namespace risk;
using namespace std;


AsmStream::AsmStream(ostream &_os, string _holder) : os(_os), holder(std::move(_holder)) {}

AsmStream::AsmStream(ostream &_os) : os(_os), holder("t0") {}

AsmStream &AsmStream::j(const string &label) {
    os << "  j    " << label << endl;
    return *this;
}

AsmStream &AsmStream::call(const string &fun) {
    os << "  call " << fun << endl;
    return *this;
}

AsmStream &AsmStream::li(const string &dst, int imm) {
    os << "  li   " << dst << ", " << imm << endl;
    return *this;
}

AsmStream &AsmStream::la(const string &dst, const string &sym) {
    os << "  la   " << dst << ", " << sym << endl;
    return *this;
}

AsmStream &AsmStream::mv(const string &dst, const string &src) {
    if (src != dst) {
        os << "  mv   " << dst << ", " << src << endl;
    }
    return *this;
}

AsmStream &AsmStream::op(const string &op, const string &dst, const string &src) {
    os << "  " << op << " " << dst << ", " << src << endl;
    return *this;
}

AsmStream &AsmStream::op(const string &op, const string &dst, const string &lhs, const string &rhs) {
    os << "  " << op << " " << dst << ", " << lhs << ", " << rhs << endl;
    return *this;
}

AsmStream &AsmStream::addi(const string &dst, const string &src, int imm) {
    if (isIMM12(imm)) {
        os << "  addi " << dst << ", " << src << ", " << imm << endl;
    } else {
        li(holder, imm);
        os << "  add  " << dst << ", " << src << ", " << holder << endl;
    }
    return *this;
}

AsmStream &AsmStream::slli(const string &dst, const string &src, int imm) {
    os << "  slli " << dst << ", " << src << ", " << imm << endl;
    return *this;
}

AsmStream &AsmStream::muli(const string &dst, const string &src, int imm) {
    if (imm == 0) {
        mv(dst, "x0");
    } else if (imm > 0 && (imm & (imm - 1)) == 0) {
        unsigned int shift = 0;
        int tmpImm = imm >> 1;
        while (tmpImm != 0) {
            shift++;
            tmpImm >>= 1;
        }
        slli(dst, src, (int) shift);
    } else {
        li(holder, imm);
        op("mul ", dst, src, holder);
    }
    return *this;
}

AsmStream &AsmStream::sw(const string &src, const string &addr, int offset) {
    if (isIMM12(offset)) {
        os << "  sw   " << src << ", " << offset << "(" << addr << ")" << endl;
    } else {
        addi(holder, addr, offset);
        os << "  sw   " << src << ", 0(" << holder << ")" << endl;
    }
    return *this;
}

AsmStream &AsmStream::lw(const string &dst, const string &addr, int offset) {
    if (isIMM12(offset)) {
        os << "  lw   " << dst << ", " << offset << "(" << addr << ")" << endl;
    } else {
        addi(holder, addr, offset);
        os << "  lw   " << dst << ", 0(" << holder << ")" << endl;
    }
    return *this;
}

AsmStream &AsmStream::bnez(const string &cond, const string &label) {
    os << "  bnez " << cond << ", " << label << endl;
    return *this;
}

