//
// Created by 李臻 on 2022/5/9.
//

#pragma once

#include <koopa.h>
#include <string>
#include <optional>
#include "type.h"
#include "operand.h"

#ifndef __APPLE__
#include <cassert>
#endif

#define for_reversed(items) for(auto it = items.rbegin(); it != items.rend(); it++)

using namespace std;

namespace ir {

    typedef pair<uint, uint> SizeRecord;

    enum class InitializerKind {
        Const,
        Value,
        List,
    };


    struct Initializer {
        InitializerKind kind;
        int constData;
        string valueData;
        vector<Initializer> listData;

        explicit Initializer(int _constData);

        explicit Initializer(string _valueData);

        explicit Initializer(const Operand &op);

        explicit Initializer(vector<Initializer> _listData);

        Initializer normalizeTo(const unique_ptr<Type> &type);

        static Initializer normalizeTo(const vector<Initializer> &inits, const vector<SizeRecord> &sizes);

        static void merge(vector<vector<Initializer>> &unified, const vector<SizeRecord> &sizes);

        friend ostream &operator<<(ostream &os, const Initializer &init);
    };

    class GlobalValue {
    public:
        string ident;
        unique_ptr<Type> type;
        optional<Initializer> init;

        explicit GlobalValue(const string &_ident, const Type &_type);

        explicit GlobalValue(const string &_ident, const Type &_type, const Initializer &_init);

        friend ostream &operator<<(ostream &os, const GlobalValue &val);
    };


    enum class EntityValueKind {
        Integer,
        ZeroInit,
        Undef,
        Aggregate,
        FuncArgRef,
        BlockArgRef,
        Alloc,
        GlobalAlloc,
        Load,
        Store,
        GetPtr,
        GetElemPtr,
        Binary,
        Branch,
        Jump,
        Call,
        Return
    };


    enum class ValueKind {
        Value,
        Const
    };

    struct Value {
    public:

        Value(int constData);

        /**
         * insert with @
         * @param valueData without @
         */
        Value(string &valueData, const unique_ptr<Type> &type);

        Value(const Value& other);


        ValueKind kind;
        int constData{};
        string valueData;

        unique_ptr<Type> type;
    };


}
