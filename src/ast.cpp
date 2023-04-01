//
// Created by 李臻 on 2022/4/22.
//

#include "ast.h"

#include <memory>

#define DEF_INIT(AST) AST :: AST (){} \
    AST::~AST() {}                    \
    ostream & AST ::Print(ostream &os) const { \
    os << #AST <<" {}\n";\
    return os;\
    }


#define DEF_INIT_IDENT(AST, IDENT) AST :: AST (){} \
    AST::~AST() {}                    \
    ostream & AST ::Print(ostream &os) const { \
    os << #AST <<" { " << IDENT << " }\n";\
    return os;\
    }

#define DEF_INIT_(AST, MEMBER1) AST :: AST (){} \
    AST::~AST() {}                    \
    ostream & AST ::Print(ostream &os) const { \
    os << #AST <<" {\n";                          \
    if(MEMBER1) MEMBER1->Print(os);                                \
    return os << "}\n";\
    }

#define DEF_INIT__(AST, MEMBER1, MEMBER2) AST :: AST (){} \
    AST::~AST() {}                    \
    ostream & AST ::Print(ostream &os) const { \
    os << #AST <<" {\n";                          \
    if(MEMBER1) MEMBER1->Print(os);                                \
    if(MEMBER2) MEMBER2->Print(os);                                \
    return os << "}\n";\
    }

template<typename T>
ostream &PrintEnum(T kind, ostream &os) {
    return os << static_cast<typename underlying_type<T>::type>(kind) << ", ";
}


BlockAST::BlockAST() {
    items = vector<unique_ptr<BlockItemAST>>();
}


ostream &BaseAST::Print(ostream &os) const {
    os << "BaseAST {}\n";
    return os;
}

ostream &CompUnitAST::Print(ostream &os) const {
    os << "CompUnitAST {\n";
    for (const auto &item: items) {
        item->Print(os);
    }
    os << "}\n";
    return os;
}

void CompUnitAST::setGlobalItems(vector<BaseAST *> *asts) {
    items = vector<unique_ptr<GlobalItemAST>>();
    for (const auto &ast: *asts) {
        items.push_back(cast_ast<GlobalItemAST>(ast));
    }
}

ostream &FuncDefAST::Print(ostream &os) const {
    os << "FuncDefAST {\n";
    funcHeader->Print(os);
    block->Print(os);
    os << "}\n";
    return os;
}

void FuncDefAST::setFuncFParams(vector<BaseAST *> *asts) {
    funcFParams = vector<unique_ptr<FuncFParamAST>>();
    for (const auto &ast: *asts) {
        funcFParams.push_back(cast_ast<FuncFParamAST>(ast));
    }
}


ostream &BlockAST::Print(ostream &os) const {
    os << "BlockAST {\n";
    for (const auto &it: items) {
        it->Print(os);
    }
    os << "}\n";
    return os;
}

void BlockAST::setBlockItems(vector<BaseAST *> *asts) {
    for (const auto &ast: *asts) {
        items.push_back(cast_ast<BlockItemAST>(ast));
    }
}


ostream &StmtAST::Print(ostream &os) const {
    os << "StmtAST {\n";
    PrintEnum(kind, os);
    os << endl;
    if (lVal) {
        lVal->Print(os);
    }
    if (exp) {
        exp->Print(os);
    }
    if (block) {
        block->Print(os);
    }
    if (ifStmt) {
        ifStmt->Print(os);
        if (elseStmt)
            elseStmt->Print(os);
    }
    if (whileStmt) {
        whileStmt->Print(os);
    }
    os << "}\n";
    return os;
}

StmtAST::StmtAST() {

}


ostream &DeclAST::Print(ostream &os) const {
    os << "DeclAST {\n";
    PrintEnum(kind, os);
    if (kind == DeclKind::Const)
        constDecl->Print(os);
    else
        varDecl->Print(os);
    os << "}\n";
    return os;
}

ostream &BlockItemAST::Print(ostream &os) const {
    if (kind == BlockItemKind::Decl) {
        decl->Print(os);
    } else {
        stmt->Print(os);
    }
    return os;
}

#ifdef __APPLE__

unique_ptr<char> make_tabs(int _count) {
    int count = _count > 0 ? _count : 0;
    char *indent = new char[count + 1];

    memset(indent, '\t', count);
    indent[count] = '\0';
    return unique_ptr<char>(indent);
}

string BaseAST::Formatted() const {
    stringstream ss, formatted;
    Print(ss);
    int indent = 0;
    string line;
    while (std::getline(ss, line)) {
        if (*(line.end() - 1) == '{') {
            formatted << make_tabs(indent) << line << endl;
            indent++;
        } else if (*line.begin() == '}') {
            indent--;
            formatted << make_tabs(indent) << line << endl;
        } else {
            formatted << make_tabs(indent) << line << endl;
        }
    }
    return formatted.str();
}
#endif

ExpAST::ExpAST() {

}

ostream &ExpAST::Print(ostream &os) const {
    os << "ExpAST {\n";
    lOrExp->Print(os);
    os << "}\n";
    return os;
}

PrimaryExpAST::PrimaryExpAST() {

}

ostream &PrimaryExpAST::Print(ostream &os) const {
    os << "PrimaryAST {\n";
    switch (kind) {
        case PrimaryExpKind::Number:
            os << number << endl;
            break;
        case PrimaryExpKind::Exp:
            exp->Print(os);
            break;
        case PrimaryExpKind::LVal:
            lVal->Print(os);
            break;
        default:
            os << "Unhandled branch" << endl;
    }

    return os << "}\n";
}


void syntaxTest() {

}


void ConstDeclAST::setConstDefs(vector<BaseAST *> *asts) {
    constDefs = vector<unique_ptr<ConstDefAST>>();
    for (const auto &ast: *asts) {
        constDefs.push_back(cast_ast<ConstDefAST>(ast));
    }
}

DEF_INIT(ConstDefAST)

void ConstDefAST::setDims(vector<BaseAST *> *asts) {
    dimConstExps = vector<unique_ptr<ConstExpAST>>();
    if (asts) {
        for (auto const &ast: *asts) {
            dimConstExps.push_back(cast_ast<ConstExpAST>(ast));
        }
    }
}

void VarDefAST::setDims(vector<BaseAST *> *asts) {
    dimConstExps = vector<unique_ptr<ConstExpAST>>();
    if (asts) {
        for (auto const &ast: *asts) {
            dimConstExps.push_back(cast_ast<ConstExpAST>(ast));
        }
    }
}

DEF_INIT(ConstExpAST)

DEF_INIT(ConstDeclAST)

DEF_INIT(ConstInitValAST)

void ConstInitValAST::setDims(vector<BaseAST *> *asts) {
    dimConstInitVals = vector<unique_ptr<ConstInitValAST>>();
    if (asts)
        for (auto const &ast: *asts) {
            dimConstInitVals.push_back(cast_ast<ConstInitValAST>(ast));
        }
}

DEF_INIT__(RelExpAST, add, rel)

DEF_INIT__(EqExpAST, rel, eq)

DEF_INIT__(LOrExpAST, lOr, lAnd)

DEF_INIT__(LAndExpAST, lAnd, eq)

DEF_INIT_IDENT(LValAST, ident)

void LValAST::setDims(vector<BaseAST *> *asts) {
    dimExps = vector<unique_ptr<ExpAST>>();
    if (asts) {
        for (auto const &ast: *asts) {
            dimExps.push_back(cast_ast<ExpAST>(ast));
        }
    }
}

DEF_INIT_IDENT(FuncHeaderAST, ident)

DEF_INIT__(UnaryExpAST, primary, unary)

void UnaryExpAST::setFuncRParams() {
    funcRParams = vector<unique_ptr<ExpAST>>();
}

void UnaryExpAST::setFuncRParams(vector<BaseAST *> *asts) {
    funcRParams = vector<unique_ptr<ExpAST>>();
    for (const auto &ast: *asts) {
        funcRParams.push_back(cast_ast<ExpAST>(ast));
    }
}

DEF_INIT__(AddExpAST, add, mul)

DEF_INIT__(MulExpAST, mul, unary)

DEF_INIT_(InitValAST, exp)

void InitValAST::setDims(vector<BaseAST *> *asts) {
    dimInitVals = vector<unique_ptr<InitValAST>>();
    if (asts)
        for (const auto &ast: *asts) {
            dimInitVals.push_back(cast_ast<InitValAST>(ast));
        }
}

DEF_INIT_IDENT(VarDefAST, ident)

VarDeclAST::VarDeclAST() {}

VarDeclAST::~VarDeclAST() {}

void VarDeclAST::setVarDefs(vector<BaseAST *> *defs) {
    varDefs = vector<unique_ptr<VarDefAST>>();
    for (const auto &ast: *defs) {
        varDefs.push_back(cast_ast<VarDefAST>(ast));
    }
}

ostream &VarDeclAST::Print(ostream &os) const {
    os << "VarDeclAST {\n";
    for (const auto &def: varDefs) {
        def->Print(os);
    }
    return os << "}\n";
}


DEF_INIT_IDENT(FuncFParamAST, ident)

void FuncFParamAST::setDims(vector<BaseAST *> *asts) {
    dimConstExps = vector<unique_ptr<ConstExpAST>>();
    if (asts) {
        for (auto const &ast: *asts) {
            dimConstExps->push_back(cast_ast<ConstExpAST>(ast));
        }
    }
}

GlobalItemAST::GlobalItemAST() {}

GlobalItemAST::~GlobalItemAST() {}

ostream &GlobalItemAST::Print(ostream &os) const {
    os << "GlobalItemAST {\n";
    if (decl) decl->Print(os);
    else funcDef->Print(os);
    return os << "}\n";
}