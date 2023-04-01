//
// Created by 李臻 on 2022/5/18.
//

#include "type.h"

#include <memory>
#include <iostream>

using namespace ir;


#define for_reversed(items) for(auto it = items.rbegin(); it != items.rend(); it++)


ir::Type::Type(TypeKind _kind) {
    assert(_kind == TypeKind::Int || _kind == TypeKind::Unit);
    kind = _kind;
    arraySize = 0;
}

ir::Type::Type(TypeKind _kind, const unique_ptr<Type> _baseType) {
    assert(_kind == TypeKind::Pointer);
    kind = _kind;
    baseType = unique_ptr<Type>(_baseType->clone());
    arraySize = 0;
}

ir::Type::Type(TypeKind _kind, const unique_ptr<Type> _baseType, const unsigned int _arraySize) {
    assert(_kind == TypeKind::Array);
    kind = _kind;
    baseType = unique_ptr<Type>(_baseType->clone());
    arraySize = _arraySize;
}

ir::Type::Type(TypeKind _kind, const vector<Type> &_params, const unique_ptr<Type> _retType) {
    assert(_kind == TypeKind::Function);
    kind = _kind;
    retType = unique_ptr<Type>(_retType->clone());
    params = vector<unique_ptr<Type>>();
    arraySize = 0;
    for (const auto &item: _params) {
        params.emplace_back(item.clone());
    }
}

ir::Type::Type(TypeKind _kind, const vector<unique_ptr<Type>> &_params, const unique_ptr<Type> _retType) {
    assert(_kind == TypeKind::Function);
    kind = _kind;
    retType = unique_ptr<Type>(_retType->clone());
    params = vector<unique_ptr<Type>>();
    arraySize = 0;
    for (const auto &item: _params) {
        params.emplace_back(item->clone());
    }
}

ostream &ir::operator<<(ostream &os, const Type &ty) {
    switch (ty.kind) {
        case TypeKind::Int: {
            return os << "i32";
        }
        case TypeKind::Unit: {
            return os << "unit";
        }
        case TypeKind::Pointer: {
            return os << "*" << *ty.baseType;
        }
        case TypeKind::Array: {
            return os << "[" << *ty.baseType << ", " << ty.arraySize << "]";
        }
        case TypeKind::Function: {
            os << "(";
            for (auto it = ty.params.begin(); it != ty.params.end(); it++) {
                if (it != ty.params.begin()) { os << ", "; }
                os << **it;
            }
            if (ty.retType->kind == TypeKind::Unit) { os << ")"; }
            else {
                os << "): " << *ty.retType;
            }
            return os;
        }
    }
}


Type::Type(const Type &other) {
    switch (other.kind) {
        case TypeKind::Int: {
            kind = TypeKind::Int;
            return;
        }
        case TypeKind::Unit: {
            kind = TypeKind::Unit;
            return;
        }
        case TypeKind::Array: {
            kind = TypeKind::Array;
            arraySize = other.arraySize;
            baseType = unique_ptr<Type>(other.baseType->clone());
            return;
        }
        case TypeKind::Pointer: {
            kind = TypeKind::Array;
            baseType = unique_ptr<Type>(other.baseType->clone());
            return;
        }
        case TypeKind::Function: {
            kind = TypeKind::Function;
            retType = unique_ptr<Type>(other.retType->clone());
            params = vector<unique_ptr<Type>>();
            for (const auto &item: other.params) {
                params.emplace_back(item->clone());
            }
            return;
        }
    }
}


unique_ptr<Type> Type::clone() const {
    switch (kind) {
        case TypeKind::Int:
        case TypeKind::Unit: {
            return make_unique<Type>(kind);
        }
        case TypeKind::Array: {
            return make_unique<Type>(
                TypeKind::Array,
                baseType->clone(),
                arraySize
            );
        }
        case TypeKind::Pointer: {
            return make_unique<Type>(
                TypeKind::Pointer,
                baseType->clone()
            );
        }
        case TypeKind::Function: {
            auto t = make_unique<Type>(
                TypeKind::Function,
                vector<unique_ptr<Type>>(),
                retType->clone()
            );
            for (const auto &item: params) {
                t->params.emplace_back(item->clone());
            }
            return t;
        }
    }
}


string ir::Type::toString() const {
    ostringstream ss;
    ss << *this;
    return ss.str();
}

unique_ptr<Type> ir::Type::getPointerFor(unique_ptr<Type> type) {
    return make_unique<Type>(
        TypeKind::Pointer,
        move(type)
    );
}

unique_ptr<Type> ir::Type::getArrayFor(unique_ptr<Type> type, unsigned int _size) {
    return make_unique<Type>(
        TypeKind::Array,
        move(type),
        _size
    );
}

unique_ptr<Type> ir::Type::getInt() {
    return make_unique<Type>(TypeKind::Int);
}
#undef for_reversed
