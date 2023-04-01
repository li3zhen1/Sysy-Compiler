//
// Created by 李臻 on 2022/5/21.
//

#pragma once

#include <string>
#include <variant>
#include <ostream>

using namespace std;

namespace ir {


    class Operand : public std::variant<int, std::string> {
    public:
        Operand(int value);

        Operand(string value);

        Operand();

        friend ostream &operator<<(ostream &os, const Operand &op);
    };
}
