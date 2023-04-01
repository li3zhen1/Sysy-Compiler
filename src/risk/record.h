//
// Created by 李臻 on 2022/5/21.
//

#pragma once

#include <variant>
#include <string>
#include <functional>
#include "bridge.h"
#include "asm.h"

using namespace std;


namespace risk {

    class Record : public variant<string, Slot, int, unsigned int> {
    public:
        enum RecordType {
            Global,
            Local,
            Const,
            Arg
        };

        string &asGlobal();

        Slot &asLocal();

        unsigned int &asArg();

        int &asConst();

        void read(ostream &, const string &, const string &);

        void write(ostream &, const string &);

        void writeAddr(ostream &, const string &);

        void writeSp(ostream &, const string &, unsigned int);

        static Record makeRecord(const optional<Slot> &);
    };
}
