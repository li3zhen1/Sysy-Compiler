//
// Created by 李臻 on 2022/5/5.
//

#pragma once

#include <koopa.h>
#include <unordered_map>
#include <variant>
#include "scope.h"
#include "ast.h"



using namespace std;

namespace ir {


    class Program {
        friend class Scope;

    public:

        Scope scope;

        Program();

        ~Program();

        void visit(const unique_ptr<CompUnitAST> &ast);


#define VISIT_S(AST) Operand visit(const unique_ptr<AST> &ast);
#define VISIT(AST) void visit(const unique_ptr<AST>& ast);

    private:
        VISIT(StmtAST)

        VISIT(FuncDefAST)

        VISIT(DeclAST)

        VISIT(ConstDefAST)

        VISIT_S(VarDefAST)

        VISIT_S(ExpAST)

        VISIT_S(LOrExpAST)

        VISIT_S(LAndExpAST)

        VISIT_S(EqExpAST)

        VISIT_S(PrimaryExpAST)

        VISIT_S(AddExpAST)

        VISIT_S(RelExpAST)

        VISIT_S(MulExpAST)

        VISIT_S(UnaryExpAST)

        Initializer visit(const unique_ptr<InitValAST> &ast);

        Initializer visit(const unique_ptr<ConstInitValAST> &ast);

#undef VISIT
#undef VISIT_S

        friend ostream &operator<<(ostream &os, const Program &prog);
    };

}