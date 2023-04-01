//
// Created by 李臻 on 2022/5/9.
//

#include "value.h"

#include <utility>

using namespace ir;


Initializer Initializer::normalizeTo(const unique_ptr<Type> &type) {
    vector<uint> sizes;
    vector<SizeRecord> sizeRecords;
    uint trailingSize = 1;
    Type *endPoint = type.get();

    while (true) {
        if (endPoint->kind == TypeKind::Int) {
            break;
        } else if (endPoint->kind == TypeKind::Array) {
            assert(kind == InitializerKind::List);
            sizes.emplace_back(endPoint->arraySize);
            endPoint = endPoint->baseType.get();
        }
    }

    for_reversed(sizes) {
        trailingSize *= *it;
        sizeRecords.emplace_back(*it, trailingSize);
    }

    if (kind != InitializerKind::List && sizes.empty()) {
        return *this;
    } else if (kind == InitializerKind::List && !sizes.empty()) {
        return normalizeTo(listData, sizeRecords);
    } else {
        throw runtime_error("Initializer type mismatch.");
    }
}

Initializer Initializer::normalizeTo(const vector<Initializer> &inits, const vector<SizeRecord> &sizes) {

    const uint sizeOfNormalized = sizes.size() + 1;
    auto normalized = vector<vector<Initializer>>(sizeOfNormalized);
    std::fill(normalized.begin(), normalized.end(), vector<Initializer>());
    uint _size = 0;
    for (const auto &init: inits) {
        if (_size >= sizes.rbegin()->second) {
            throw runtime_error("Initializer type mismatch.");
        }
        if (init.kind == InitializerKind::List) {

            vector<SizeRecord> nextSizeRecord;
            optional<uint> firstFilled = nullopt;
            for (uint i = 0; i < sizeOfNormalized; i++) {
                if (!normalized[i].empty()) {
                    firstFilled = i;
                    break;
                }
            }
            if (firstFilled) {
                if (firstFilled.value() == 0) {
                    throw runtime_error("Initializer type mismatch.");
                } else {
                    nextSizeRecord = vector<SizeRecord>(
                        sizes.begin(),
                        sizes.begin() + firstFilled.value()
                    );
                }
            } else {
                nextSizeRecord = vector<SizeRecord>(
                    sizes.begin(),
                    sizes.end() - 1
                );
            }
            normalized[nextSizeRecord.size()].push_back(
                normalizeTo(init.listData, nextSizeRecord)
            );
            merge(normalized, sizes);
            _size += nextSizeRecord.rbegin()->second;
        } else {
            normalized[0].push_back(init);
            merge(normalized, sizes);
            _size++;
        }
    }
    while (_size < sizes.rbegin()->second) {
        normalized[0].push_back(Initializer{0});
        merge(normalized, sizes);
        _size++;
    }
    return *normalized.rbegin()->rbegin();
}

void Initializer::merge(vector<vector<Initializer>> &normalized, const vector<SizeRecord> &sizes) {
    const uint sizeCount = sizes.size();
    for (uint i = 0; i < sizeCount; i++) {
        if (normalized[i].size() == sizes[i].first) {
            Initializer init = Initializer{
                normalized[i]
            };
            init.listData = normalized[i];
            normalized[i] = vector<Initializer>();
            normalized[i + 1].push_back(init);
        }
    }
}


Value::Value(int _constValue) : constData(_constValue) {
    kind = ValueKind::Const;
    type = Type::getInt();
}

Value::Value(string &_valueData, const unique_ptr<Type> &_type) : valueData(_valueData) {
    kind = ValueKind::Value;
    type = unique_ptr<Type>(_type->clone());
}

Value::Value(const Value &other) {
    kind = other.kind;
    if (kind == ValueKind::Const) {
        type = Type::getInt();
        constData = other.constData;
    } else {
        valueData = other.valueData;
        type = (other.type->clone());
    }
}

ostream &ir::operator<<(ostream &os, const GlobalValue &val) {
    os << "global " << val.ident << " = alloc " << *val.type << ", ";
    if (val.init) {
        return os << val.init.value() << endl;
    } else {
        return os << "zeroinit" << endl;
    }
}

GlobalValue::GlobalValue(const string &_ident, const Type &_type) {
    ident = _ident;
    type = _type.clone();
}

GlobalValue::GlobalValue(const string &_ident, const Type &_type, const Initializer &_init) {
    ident = _ident;
    type = _type.clone();
    init = _init;
}

ostream &ir::operator<<(ostream &os, const Initializer &init) {
    // reshaped
    switch (init.kind) {
        case InitializerKind::Const: {
            return os << init.constData;
        }
        case InitializerKind::Value: {
            return os << init.valueData;
        }
        case InitializerKind::List: {
            os << "{";
            for (auto it = init.listData.begin();
                 it != init.listData.end();
                 it++) {
                if (it != init.listData.begin()) {
                    os << ", ";
                }
                os << *it;
            }
            return os << "}";
        }
    }
}

Initializer::Initializer(int _constData) {
    constData = _constData;
    kind = InitializerKind::Const;
}

Initializer::Initializer(string _valueData) {
    valueData = std::move(_valueData);
    kind = InitializerKind::Value;
}

Initializer::Initializer(const Operand &op) {
    if (op.index() == 0) {
        constData = get<int>(op);
        kind = InitializerKind::Const;
    } else {
        valueData = get<string>(op);
        kind = InitializerKind::Value;
    }
}

Initializer::Initializer(vector<Initializer> _listData) {
    listData = std::move(_listData);
    kind = InitializerKind::List;
}


