//
// Created by 李臻 on 2022/5/9.
//

#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include "block.h"
#include "type.h"

#ifndef __APPLE__
#include <cassert>
#endif

using namespace std;


namespace ir {

    class FunctionSignature {
    public:
        vector<unique_ptr<Type>> params;
        unique_ptr<Type> returnType;

        FunctionSignature(const vector<pair<string, unique_ptr<Type>>> &params,
                          const Type &returnType);

        FunctionSignature(const Type &_returnType);

        FunctionSignature(const vector<unique_ptr<Type>> &params,
                          const Type &returnType);

        FunctionSignature(const Type &_returnType, const Type &_paramType);
    };

    class GlobalDecl {
    public:
        string ident;
        unique_ptr<FunctionSignature> signature;
        GlobalDecl(const string& str, const FunctionSignature& funSignature);
        friend ostream &operator<<(ostream& os, const GlobalDecl& decl);
    };



    class Function {


        friend class Scope;

    protected:

        vector<BasicBlock> blocks;

        vector<Instruction> leadingAlloc;

        vector<pair<string, unique_ptr<Type>>> paramTypes;

        unique_ptr<BasicBlock> block;


        unordered_map<string, unsigned int> namedSymbolCount;
        unsigned int anonymousSymbolCount;

    public:

        string ident;
        FuncTypeKind returnType;

        BasicBlock *workingBlock;


        explicit Function(const FuncHeaderAST &header);

        string assignNamedSymbol(const string &_name);

        string assignAnonymousSymbol();

        void setParamTypes(vector<pair<string, unique_ptr<Type>>> &_paramTypes);


        BasicBlock &getEntryBlock();

        BasicBlock &getCurrentBlock();

        void setWorkingBlock(BasicBlock &working);

        void createAlloc(const string &symbol, const Type &type);

        void commitFunction();

        BasicBlock &createNewBlock(const string &blockName);

        friend ostream &operator<<(ostream &os, const Function &fun);

        unique_ptr<FunctionSignature> getSignature() const;
    };

}
