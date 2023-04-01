//
// Created by 李臻 on 2022/5/21.
//

#include "record.h"

using namespace risk;


string &Record::asGlobal() {
    return get<string>(*this);
}

Slot &Record::asLocal() {
    return get<Slot>(*this);
}

unsigned int &Record::asArg() {
    return get<unsigned int>(*this);
}

int &Record::asConst() {
    return get<int>(*this);
}

void Record::write(ostream &os, const string &reg) {
    auto inst = AsmStream(os, reg);
    switch (index()) {
        case Global: {
            inst.la(reg, asGlobal())
                .lw(reg, reg, 0);
            break;
        }
        case Local: {
            inst.lw(reg, "sp", (int) asLocal().offset);
            break;
        }
        case Const: {
            inst.li(reg, asConst());
            break;
        }
        default: {
            throw runtime_error("Unexpected branch");
        }
    }
}

void Record::writeAddr(ostream &os, const string &reg) {
    auto inst = AsmStream(os, reg);
    if (index() == Global)
        inst.la(reg, asGlobal());
    else if (index() == Local)
        inst.addi(reg, "sp", (int) asLocal().offset);
}

void Record::writeSp(ostream &os, const string &reg, unsigned int spOffset) {
    auto inst = AsmStream(os, reg);
    if (index() == Arg) {
        if (asArg() < 8)
            inst.mv(reg, "a" + to_string(asArg()));
        else
            inst.lw(reg, "sp", (int) (spOffset + (asArg() - 8) * 4));
    }
}

void Record::read(ostream &os, const string &reg, const string &holder) {
    auto inst = AsmStream(os, holder);
    switch (index()) {
        case Global: {
            inst.la(holder, asGlobal())
                .sw(reg, holder, 0);
            return;
        }
        case Local: {
            inst.sw(reg, "sp", (int) asLocal().offset);
            return;
        }
        case Arg: {
            const unsigned int arg = asArg();
            if (arg < 8)
                inst.mv("a" + to_string(arg), reg);
            else
                inst.sw(reg, "sp", (int) ((arg - 8) * 4));
            return;
        }
    }
}

Record risk::Record::makeRecord(const optional<Slot> &slot) {
    if (slot) return {slot.value()};
    else return Record{0};
}