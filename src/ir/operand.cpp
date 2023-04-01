//
// Created by 李臻 on 2022/5/21.
//

#include "operand.h"
#include <ostream>

ostream &ir::operator<<(ostream &os, const Operand &op) {
    if (op.index() == 0) {
        return os << get<int>(op);
    } else {
        return os << get<string>(op);
    }
}

ir::Operand::Operand(int value) {
    this->emplace<int>(value);
}

ir::Operand::Operand(string value) {
    this->emplace<string>(value);
}

ir::Operand::Operand() {
    this->emplace<int>(0);
}