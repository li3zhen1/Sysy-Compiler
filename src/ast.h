//
// Created by 李臻 on 2022/4/22.
//


// Notice that all arrays are reversed

#pragma once

#include <memory>
#include <iostream>
#include <sstream>
#include <vector>
#include <optional>
#include <optional>



#define INSTALL_PRINT ostream &Print(ostream &os) const override;

#define DECL_AST(AST) explicit AST(); \
    virtual ~AST ();               \
    ostream &Print(ostream &os) const;\

using namespace std;


enum class UnaryOpKind {
    Neg, LNot
};
enum class MulOpKind {
    Mul = '*', Div = '/', Mod = '%'
};

enum class AddOpKind {
    Add = '+', Sub = '-',
};

enum class AddExpKind {
    Mul, AddMul
};

enum class RelOpKind {
    Lt, Gt, Le, Ge,
};

enum class EqOpKind {
    Eq, Neq,
};


enum class FuncTypeKind {
    Void, Int,
};

enum class BlockItemKind {
    Decl, Stmt,
};

enum class PrimaryExpKind {
    Exp, LVal, Number,
};

enum class UnaryExpKind {
    Primary, Call, Unary,
};

enum class MulExpKind {
    Unary, MulUnary
};

enum class DeclKind {
    Const, Var,
};

enum class InitValKind {
    Exp, List,
};

enum class ConstInitValKind {
    Exp, List,
};

enum class StmtKind {
    Assign, ExpStmt, Block,
    If, While, Break, Continue, Return,
};


class BaseAST {
public:
    BaseAST() = default;

    virtual ~BaseAST() = default;

    virtual ostream &Print(ostream &os) const = 0;

#ifdef __APPLE__
    string Formatted() const;
#endif
};

template<typename T>
unique_ptr<T> cast_ast(BaseAST *raw) {
    return unique_ptr<T>((T *) raw);
}

enum class GlobalItemKind {
    Decl, FuncDef
};

class DeclAST;
class FuncDefAST;

class GlobalItemAST : public BaseAST {
public:
    DECL_AST(GlobalItemAST)
    GlobalItemKind kind;
    unique_ptr<FuncDefAST> funcDef;
    unique_ptr<DeclAST> decl;
};


class CompUnitAST : public BaseAST {
public:
    vector<unique_ptr<GlobalItemAST>> items;
    void setGlobalItems(vector<BaseAST*> * asts);
    INSTALL_PRINT
};

class FuncHeaderAST;
class BlockAST;
class ConstExpAST;


class FuncFParamAST: public BaseAST {
public:
    string ident;
    optional<vector<unique_ptr<ConstExpAST>>> dimConstExps;
    void setDims(vector<BaseAST*> *asts);
    DECL_AST(FuncFParamAST)
};


class FuncDefAST : public BaseAST {
public:
    unique_ptr<FuncHeaderAST> funcHeader;
    vector<unique_ptr<FuncFParamAST>> funcFParams;
    unique_ptr<BlockAST> block;

    void setFuncFParams(vector<BaseAST*>* asts);

    INSTALL_PRINT
};

class ConstDeclAST;

class VarDeclAST;

class DeclAST : public BaseAST {
public:
    DeclKind kind;
    unique_ptr<ConstDeclAST> constDecl;
    unique_ptr<VarDeclAST> varDecl;

    INSTALL_PRINT
};

class ExpAST;

class LValAST;

class BlockAST;

class StmtAST : public BaseAST {
public:
    StmtKind kind;

    /**
     * Optional!!
     */
    unique_ptr<ExpAST> exp; // nullable
    unique_ptr<LValAST> lVal;
    unique_ptr<BlockAST> block;

    unique_ptr<StmtAST> ifStmt;
    unique_ptr<StmtAST> elseStmt;
    unique_ptr<StmtAST> whileStmt;

    explicit StmtAST();

    INSTALL_PRINT
};

class BlockItemAST : public BaseAST {
public:
    BlockItemKind kind;

    unique_ptr<DeclAST> decl;
    unique_ptr<StmtAST> stmt;


    INSTALL_PRINT
};

class BlockAST : public BaseAST {
public:
    explicit BlockAST();

    vector<unique_ptr<BlockItemAST>> items;

    void setBlockItems(vector<BaseAST *> *item);

    INSTALL_PRINT

};

class LOrExpAST;

class ExpAST : public BaseAST {
public:
    unique_ptr<LOrExpAST> lOrExp;

    explicit ExpAST();

    INSTALL_PRINT
};

class LValAST;


class PrimaryExpAST : public BaseAST {
public:
    PrimaryExpKind kind;

    explicit PrimaryExpAST();

    int number;

    unique_ptr<ExpAST> exp;

    unique_ptr<LValAST> lVal;

    INSTALL_PRINT
};

class UnaryExpAST : public BaseAST {
public:
    UnaryExpKind kind;

    unique_ptr<PrimaryExpAST> primary;
    unique_ptr<UnaryExpAST> unary;

    string ident;
    vector<unique_ptr<ExpAST>> funcRParams;

    void setFuncRParams();
    void setFuncRParams(vector<BaseAST*> * asts);

    char unaryOp;

    DECL_AST(UnaryExpAST)
};


class FuncHeaderAST : public BaseAST {
public:
    FuncTypeKind kind;
    string ident;

    DECL_AST(FuncHeaderAST);
};


class MulExpAST : public BaseAST {
public:
    MulExpKind kind;
    unique_ptr<UnaryExpAST> unary;
    unique_ptr<MulExpAST> mul;
    char op;

    DECL_AST(MulExpAST)


};

class AddExpAST : public BaseAST {
public:
    AddExpKind kind;
    char op;

    unique_ptr<AddExpAST> add;
    unique_ptr<MulExpAST> mul;

    DECL_AST(AddExpAST)
};


enum class RelExpKind {
    Add, RelAdd
};

class RelExpAST : public BaseAST {
public:
    RelExpKind kind;
    string op;
    unique_ptr<AddExpAST> add;
    unique_ptr<RelExpAST> rel;

    DECL_AST(RelExpAST)
};

enum class EqExpKind {
    Rel, EqRel
};

class EqExpAST : public BaseAST {
public:
    EqExpKind kind;
    string op;
    unique_ptr<RelExpAST> rel;
    unique_ptr<EqExpAST> eq;

    DECL_AST(EqExpAST)
};

enum class LAndExpKind {
    Eq, LAndEq
};

class LAndExpAST : public BaseAST {
public:
    LAndExpKind kind;
    unique_ptr<EqExpAST> eq;
    unique_ptr<LAndExpAST> lAnd;

    DECL_AST(LAndExpAST)
};

enum class LOrExpKind {
    LAnd, LOrLAnd
};

class LOrExpAST : public BaseAST {
public:
    LOrExpKind kind;
    unique_ptr<LAndExpAST> lAnd;
    unique_ptr<LOrExpAST> lOr;

    DECL_AST(LOrExpAST)
};


class ConstExpAST : public BaseAST {
public:
    unique_ptr<ExpAST> exp;

    DECL_AST(ConstExpAST)

};

class LValAST : public BaseAST {
public:

    string ident;
    vector<unique_ptr<ExpAST>> dimExps;
    void setDims(vector<BaseAST*>* asts);
    DECL_AST(LValAST)
};

class ConstInitValAST : public BaseAST {
public:
    ConstInitValKind kind;
    unique_ptr<ConstExpAST> constExp;
    vector<unique_ptr<ConstInitValAST>> dimConstInitVals;

    void setDims(vector<BaseAST*> * asts);
    DECL_AST(ConstInitValAST)
};

class ConstDefAST : public BaseAST {
public:
    string ident;
    unique_ptr<ConstInitValAST> constInitVal;

    vector<unique_ptr<ConstExpAST>> dimConstExps;

    void setDims(vector<BaseAST*>*asts);
    DECL_AST(ConstDefAST)
};

class ConstDeclAST : public BaseAST {
public:
    vector<unique_ptr<ConstDefAST>> constDefs;

    DECL_AST(ConstDeclAST)

    void setConstDefs(vector<BaseAST *> *defs);
};

class InitValAST : public BaseAST {
public:
    InitValKind kind;
    unique_ptr<ExpAST> exp;
    vector<unique_ptr<InitValAST>> dimInitVals;

    void setDims(vector<BaseAST*> *asts);
    DECL_AST(InitValAST)
};


class VarDefAST : public BaseAST {
public:

    string ident;
    unique_ptr<InitValAST> initVal;
    vector<unique_ptr<ConstExpAST>> dimConstExps;

    void setDims(vector<BaseAST*>* asts);
    DECL_AST(VarDefAST)
};


class VarDeclAST : public BaseAST {
public:
    vector<unique_ptr<VarDefAST>> varDefs;

    DECL_AST(VarDeclAST)

    void setVarDefs(vector<BaseAST *> *defs);
};




