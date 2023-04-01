//
// Created by 李臻 on 2022/5/9.
//

#include "func.h"

using namespace ir;


Function::Function(
    const FuncHeaderAST &header
) {
    ident = header.ident;
    returnType = header.kind;
    namedSymbolCount = unordered_map<string, unsigned int>();
    anonymousSymbolCount = 0;
    blocks = vector<BasicBlock>();
    leadingAlloc = vector<Instruction>();

    if (returnType == FuncTypeKind::Int) {
        auto retValueName = assignNamedSymbol("%ret");
        createAlloc(retValueName, Type{
            TypeKind::Int
        });
    }

    auto entryBlockName = assignNamedSymbol("%entry");
    auto &entryBlock = blocks.emplace_back(entryBlockName);

    workingBlock = &entryBlock;

}


BasicBlock &Function::getEntryBlock() {
    return *blocks.begin();
}

BasicBlock &Function::getCurrentBlock() {
    return *blocks.rbegin();
}

void Function::setWorkingBlock(BasicBlock &working) {
    workingBlock = &working;
}


void Function::createAlloc(const string &symbol, const Type &type) {
    leadingAlloc.emplace_back(
        symbol,
        type
    );
}

void Function::commitFunction() {
//    assert(getCurrentBlock().escape);
    if (!getCurrentBlock().escape) {
        getCurrentBlock().commitBlockByJump("%end");
    }
    auto endBlockName = assignNamedSymbol("%end");
    auto &endBlock = blocks.emplace_back(endBlockName);
    if (returnType == FuncTypeKind::Void) {
        endBlock.commitBlockByRet();
    } else {
        auto tmp = assignAnonymousSymbol();
        endBlock.pushInstruction(IrKind::Load, tmp, Operand("%ret"));
        endBlock.commitBlockByRet(tmp);
    }
}

BasicBlock &Function::createNewBlock(const string &blockName) {
    return blocks.emplace_back(blockName);
}


string Function::assignNamedSymbol(const string &_name) {
    auto it = namedSymbolCount.find(_name);
    if (it != namedSymbolCount.end()) {
        return _name + "_" + to_string(++it->second);
    } else {
        namedSymbolCount.insert(make_pair(_name, 0));
        return _name;
    }
}

string Function::assignAnonymousSymbol() {
    return "%" + to_string(anonymousSymbolCount++);
}

ostream &ir::operator<<(ostream &os, const Function &fun) {

    const auto &entry = *fun.blocks.begin();

    os << "fun @" << fun.ident << "(";

    for (auto it = fun.paramTypes.begin();
         it != fun.paramTypes.end();
         it++) {
        if (it != fun.paramTypes.begin()) {
            os << ", ";
        }
        os << it->first << ": " << *it->second;
    }

    os << ")" << (fun.returnType == FuncTypeKind::Int ? ": i32" : "") << " {\n"
       << entry.ident << ":\n";

    for (const auto &alloc: fun.leadingAlloc) {
        os << alloc;
    }
    for (const auto &inst: entry.inst) {

        os << inst;
    }
    os << *entry.escape;

    if (fun.blocks.size() > 1) {
        for (auto it = fun.blocks.begin() + 1;
             it != fun.blocks.end();
             it++) {
            if (it->isolated && it->inst.empty())continue;
            os << endl << *it;
        }
    }

    return os << "}" << endl;

}

void Function::setParamTypes(vector<pair<string, unique_ptr<Type>>> &_paramTypes) {
    paramTypes = vector<pair<string, unique_ptr<Type>>>();
    for (const auto &item: _paramTypes) {
        string itemIdent = string(item.first);
        paramTypes.emplace_back(itemIdent, item.second->clone());
    }
}


unique_ptr<FunctionSignature> Function::getSignature() const {
    auto signature = make_unique<FunctionSignature>(
        vector<unique_ptr<Type>>(),
        (returnType == FuncTypeKind::Int ? Type{TypeKind::Int} : Type{TypeKind::Unit})
    );
    for (const auto &item: paramTypes) {
        signature->params.emplace_back(item.second->clone());
    }
    return signature;
}

FunctionSignature::FunctionSignature(
    const vector<pair<string, unique_ptr<Type>>> &_params,
    const Type &_returnType) {
    returnType = unique_ptr<Type>(_returnType.clone());
    params = vector<unique_ptr<Type>>();
    for (const auto &item: _params) {
        params.emplace_back(item.second->clone());
    }
}

FunctionSignature::FunctionSignature(const Type &_returnType) {
    returnType = unique_ptr<Type>(_returnType.clone());
    params = vector<unique_ptr<Type>>();
}

FunctionSignature::FunctionSignature(const vector<unique_ptr<Type>> &_params,
                                     const Type &_returnType) {
    returnType = unique_ptr<Type>(_returnType.clone());
    params = vector<unique_ptr<Type>>();
    for (const auto &item: _params) {
        params.emplace_back(item->clone());
    }
}

FunctionSignature::FunctionSignature(const Type &_returnType, const Type &_paramType) {
    returnType = unique_ptr<Type>(_returnType.clone());
    params = vector<unique_ptr<Type>>();
    params.emplace_back(_paramType.clone());
}

GlobalDecl::GlobalDecl(const string &str, const FunctionSignature &funSignature) {
    ident = str;
    signature = (make_unique<FunctionSignature>(funSignature.params, *funSignature.returnType));
}

ostream &ir::operator<<(ostream &os, const GlobalDecl &decl) {
    os << "decl " << decl.ident << "(";
    for (auto it = decl.signature->params.begin();
         it != decl.signature->params.end();
         it++) {
        if (it != decl.signature->params.begin()) {
            os << ", ";
        }
        os << **it;
    }

    os << ")" << (decl.signature->returnType->kind == TypeKind::Int ? ": i32" : "") << endl;
    return os;
}
