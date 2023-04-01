//
// Created by 李臻 on 2022/5/9.
//

#pragma once


#include <vector>
#include <unordered_map>
#include <variant>

#include "ast.h"
#include "value.h"
#include "func.h"


namespace ir {

    typedef variant<unique_ptr<Function>, unique_ptr<GlobalValue>, unique_ptr<GlobalDecl>> GlobalItem;

    class Scope {
        vector<unordered_map<string, Value>> valueMaps;

        unordered_map<string, unsigned int> anonymousSymbolCount;

        unordered_map<string, unique_ptr<FunctionSignature>> functionSignatures;

        vector<GlobalItem> globals;

        vector<pair<const string, const string>> loopInfo;

    public:

        void captureGlobals(unique_ptr<Function> &func);

        void captureGlobals(unique_ptr<GlobalValue> &val);

        void captureGlobals(unique_ptr<GlobalDecl> &decl);

        const vector<GlobalItem> &getGlobals() const;

        unique_ptr<Function> currentFunction;

        Scope();

        void enterFunction(const FuncHeaderAST &header, const vector<unique_ptr<FuncFParamAST>> &params);

        void exitFunction();

        void enterLoop(const string &, const string &);

        void exitLoop();

        pair<const string, const string> &getLoopInfo();


#define EVAL(AST) optional<int> eval(AST& ast);

        EVAL(ExpAST)

        EVAL(LValAST)

        EVAL(PrimaryExpAST)

        EVAL(UnaryExpAST)

        EVAL(MulExpAST)

        EVAL(AddExpAST)

        EVAL(RelExpAST)

        EVAL(EqExpAST)

        EVAL(LAndExpAST)

        EVAL(LOrExpAST)

        EVAL(ConstExpAST)

#undef EVAL

        [[nodiscard]] bool isGlobal() const;

        const Value &getValue(string &ident) const;

        const FunctionSignature &getFunctionSignature(string &ident) const;

        void addValue(const string &ident, Value &value);

        void addFunctionSignature(const string &ident, unique_ptr<FunctionSignature> function);

        void enterScope();

        void exitScope();

        unique_ptr<Type> getTypeFromDims(const optional<vector<unique_ptr<ConstExpAST>>> &dims);

        void initializeStdlib();



        void initArrayOnBlock(const string &symbol, const Type& type, const Initializer& init);

    };
}
