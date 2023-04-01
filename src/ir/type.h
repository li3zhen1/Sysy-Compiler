//
// Created by 李臻 on 2022/5/18.
//

#pragma once

#include <vector>
#include <ostream>
#include <string>
#include <sstream>
#include <memory>

#ifndef __APPLE__
#include <cassert>
#endif

using namespace std;

namespace ir {



    enum class TypeKind {
        Int,
        Unit,
        Array,
        Pointer,
        Function,
    };

    struct Type {
    public:
        Type(TypeKind _kind);

        Type(TypeKind _kind, unique_ptr<Type> _baseType);

        Type(TypeKind _kind, unique_ptr<Type> _baseType, const unsigned int _arraySize);

        Type(TypeKind _kind, const vector<Type> &_params, unique_ptr<Type> _retType);

        Type(TypeKind _kind, const vector<unique_ptr<Type>> &_params, unique_ptr<Type> _retType);


        Type(const Type &other);

        TypeKind kind;
        unique_ptr<Type> baseType;
        unsigned int arraySize;
        vector<unique_ptr<Type>> params;
        unique_ptr<Type> retType;

        friend ostream &operator<<(ostream &os, const Type &ty);

        string toString() const;

        static unique_ptr<Type> getPointerFor(unique_ptr<Type> type);

        static unique_ptr<Type> getArrayFor(unique_ptr<Type> type, unsigned int _size);

        unique_ptr<Type> clone() const;

        static unique_ptr<Type> getInt();
    };

}