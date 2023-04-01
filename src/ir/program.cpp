//
// Created by 李臻 on 2022/5/5.
//

#include "program.h"

#ifndef __APPLE__
#include <cassert>
#endif

using namespace ir;

#define for_reversed(items) for(auto it = items.rbegin(); it != items.rend(); it++)
#define WORKING_BLOCK scope.currentFunction->workingBlock


Program::Program() { scope = Scope(); }

Program::~Program() {
}

void Program::visit(const unique_ptr<CompUnitAST> &ast) {
    scope.initializeStdlib();
    for_reversed(ast->items) {
        if ((*it)->kind == GlobalItemKind::Decl) {
            visit((*it)->decl);
        } else {
            visit((*it)->funcDef);
        }
    }
}

void Program::visit(const unique_ptr<DeclAST> &ast) {
    if (ast->kind == DeclKind::Const) {
        for_reversed(ast->constDecl->constDefs) {
            visit(*it);
        }
    } else {
        for_reversed(ast->varDecl->varDefs) {
            visit(*it);
        }
    }
}


void Program::visit(const unique_ptr<FuncDefAST> &ast) {
    scope.enterScope();
    scope.enterFunction(*ast->funcHeader, ast->funcFParams);
    for_reversed(ast->block->items) {
        if ((*it)->kind == BlockItemKind::Stmt) {
            visit((*it)->stmt);
        } else {
            visit((*it)->decl);
        }
    }

    scope.currentFunction->commitFunction();
    scope.exitFunction();
    scope.exitScope();
}

void Program::visit(const unique_ptr<ConstDefAST> &ast) {
    if (ast->dimConstExps.empty()) {
        if (ast->constInitVal) {
            if (ast->constInitVal->kind == ConstInitValKind::Exp) {
                auto eval = scope.eval(*(ast->constInitVal->constExp));
                auto value = Value{eval.value()};
                scope.addValue(ast->ident, value);
            } else {
                throw runtime_error("Initialization type mismatch.");
            }
        } else {
            throw runtime_error("Declaring const value without initialization.");
        }
    } else {
        unique_ptr<Type> type = make_unique<Type>(TypeKind::Int);

        for (const auto &it: ast->dimConstExps) {
            const auto res = scope.eval(*it);
            if (!res) {
                throw runtime_error("Unable to evaluate initializer of " + ast->ident + ".");
            }
            type = make_unique<Type>(
                TypeKind::Array,
                move(type),
                res.value()
            );
        }

        if (scope.isGlobal()) {
            auto symbolName = "@" + ast->ident;
            if (ast->constInitVal) {
                auto unifiedInitializer = visit(ast->constInitVal).normalizeTo(type);
                auto decl = make_unique<GlobalValue>(symbolName, *type, unifiedInitializer);
                auto symbol = Value{symbolName, type};
                scope.addValue(ast->ident, symbol);
                scope.captureGlobals(decl);
            } else {
                auto decl = make_unique<GlobalValue>(symbolName, *type);
                auto symbol = Value{symbolName, type};
                scope.addValue(ast->ident, symbol);
                scope.captureGlobals(decl);
            }
        } else {
            auto symbolName = scope.currentFunction->assignNamedSymbol("@" + ast->ident);
            scope.currentFunction->createAlloc(symbolName, *type);
            if (ast->constInitVal) {
                auto init = visit(ast->constInitVal).normalizeTo(type);
                scope.initArrayOnBlock(symbolName, *type, init);
            }
            auto symbol = Value{symbolName, type};
            scope.addValue(ast->ident, symbol);
        }
    }
}


Initializer Program::visit(const unique_ptr<InitValAST> &ast) {
    if (ast->kind == InitValKind::Exp) {
        if (scope.isGlobal()) {
            const auto evaluated = scope.eval(*ast->exp);
            if (!evaluated) {
                throw runtime_error("Unable to evaluate at ConstInitValAST.");
            }
            return Initializer{evaluated.value()};
        } else {
            return Initializer{visit(ast->exp)};
        }
    } else {
        vector<Initializer> children = vector<Initializer>();
        for_reversed(ast->dimInitVals) {
            children.push_back(visit(*it));
        }
        return Initializer{children};
    }
}

Initializer Program::visit(const unique_ptr<ConstInitValAST> &ast) {
    if (ast->kind == ConstInitValKind::Exp) {
        const auto evaluated = scope.eval(*ast->constExp);
        if (!evaluated) {
            throw runtime_error("Unable to evaluate at ConstInitValAST.");
        }
        return Initializer{evaluated.value()};
    } else {
        vector<Initializer> children = vector<Initializer>();
        for_reversed(ast->dimConstInitVals) {
            children.push_back(visit(*it));
        }
        return Initializer{children};
    }
}

Operand Program::visit(const unique_ptr<VarDefAST> &ast) {
    if (ast->dimConstExps.empty()) {
        if (ast->initVal) {
            if (ast->initVal->kind == InitValKind::Exp) {
                if (scope.isGlobal()) {
                    auto symbolName = "@" + ast->ident;
                    auto value = scope.eval(*ast->initVal->exp);
                    auto symbol = Value{symbolName, Type::getInt()};
                    scope.addValue(ast->ident, symbol);
                    if (!value) {
                        throw runtime_error("Unable to evaluate " + ast->ident + ".");
                    }
                    auto globalValue = make_unique<GlobalValue>(symbolName, *symbol.type, Initializer(value.value()));
                    scope.captureGlobals(globalValue);
                    return symbolName;
                } else {
                    auto symbolName = scope.currentFunction->assignNamedSymbol("@" + ast->ident);
                    scope.currentFunction->createAlloc(symbolName, Type{TypeKind::Int});
                    auto symbol = Value{symbolName, Type::getInt()};
                    auto value = visit(ast->initVal->exp);
                    scope.addValue(ast->ident, symbol);
                    WORKING_BLOCK->pushInstruction(IrKind::Store, value, symbol.valueData);
                    return symbolName;
                }
            } else {
                throw runtime_error("Initialize type mismatch.");
            }
        } else {
            if (scope.isGlobal()) {
                auto symbolName = "@" + ast->ident;
                auto symbol = Value{symbolName, Type::getInt()};
                scope.addValue(ast->ident, symbol);
                auto globalValue = make_unique<GlobalValue>(symbolName, *symbol.type);
                scope.captureGlobals(globalValue);
                return symbolName;
            } else {
                auto symbolName = scope.currentFunction->assignNamedSymbol("@" + ast->ident);
                scope.currentFunction->createAlloc(symbolName, Type{TypeKind::Int});
                auto symbol = Value{symbolName, Type::getInt()};
                scope.addValue(ast->ident, symbol);
                return symbolName;
            }
        }
    } else {
        unique_ptr<Type> type = make_unique<Type>(TypeKind::Int);

        for (const auto &it: ast->dimConstExps) {
            const auto res = scope.eval(*it);
            if (!res) {
                throw runtime_error("Unable to evaluate initializer of " + ast->ident + ".");
            }
            type = make_unique<Type>(
                TypeKind::Array,
                move(type),
                res.value()
            );
        }

        if (scope.isGlobal()) {
            auto symbolName = "@" + ast->ident;

            if (ast->initVal) {
                auto unifiedInitializer = visit(ast->initVal).normalizeTo(type);
                auto decl = make_unique<GlobalValue>(symbolName, *type, unifiedInitializer);

                auto symbol = Value{symbolName, type};
                scope.addValue(ast->ident, symbol);
                scope.captureGlobals(decl);
            } else {
                auto decl = make_unique<GlobalValue>(symbolName, *type);
                auto symbol = Value{symbolName, type};
                scope.addValue(ast->ident, symbol);
                scope.captureGlobals(decl);
            }
            return symbolName;

        } else {
            auto symbolName = scope.currentFunction->assignNamedSymbol("@" + ast->ident);

            scope.currentFunction->createAlloc(symbolName, *type);

            if (ast->initVal) {
                auto init = visit(ast->initVal).normalizeTo(type);
                scope.initArrayOnBlock(symbolName, *type, init);
            }

            auto symbol = Value{symbolName, type};
            scope.addValue(ast->ident, symbol);
            return symbolName;

        }
    }
}


void Program::visit(const unique_ptr<StmtAST> &ast) {
    switch (ast->kind) {
        case StmtKind::Return: {
            if (ast->exp) {
                auto retVal = visit(ast->exp);
                WORKING_BLOCK->pushInstruction(IrKind::Store, retVal, Operand("%ret"));
            }
            WORKING_BLOCK->commitBlockByJump("%end");
            auto nextBlockName = scope.currentFunction->assignAnonymousSymbol();
            auto &nextBlock = scope.currentFunction->createNewBlock(nextBlockName);
            nextBlock.isolated = true;
            return scope.currentFunction->setWorkingBlock(nextBlock);
        }
        case StmtKind::Assign: {
            const auto &lVal = scope.getValue(ast->lVal->ident);
            auto rVal = visit(ast->exp);
            if (ast->lVal->dimExps.empty()) {
                if (lVal.kind == ValueKind::Value) {
                    WORKING_BLOCK->pushInstruction(IrKind::Store, rVal, lVal.valueData);
                    return;
                } else {
                    throw runtime_error("Assign to const.\n");
                }
            } else {
                Operand ptr = lVal.valueData;
                Type *currType = lVal.type.get();
                for_reversed(ast->lVal->dimExps) {
                    auto assigned = scope.currentFunction->assignAnonymousSymbol();
                    if (currType->kind == TypeKind::Pointer) {
                        auto container = scope.currentFunction->assignAnonymousSymbol();
                        WORKING_BLOCK->pushInstruction(
                            IrKind::Load,
                            container,
                            ptr
                        );
                        WORKING_BLOCK->pushInstruction(
                            IrKind::GetPtr,
                            assigned,
                            container,
                            visit(*it)
                        );
                    } else if (currType->kind == TypeKind::Array) {
                        WORKING_BLOCK->pushInstruction(
                            IrKind::GetElemPtr,
                            assigned,
                            ptr,
                            visit(*it)
                        );
                    }
                    ptr = assigned;
                    currType = currType->baseType.get();
                }
                WORKING_BLOCK->pushInstruction(IrKind::Store, rVal, ptr);
                return;
            }
        }
        case StmtKind::ExpStmt: {
            if (ast->exp) {
                visit(ast->exp);
            }
            return;//scope.currentFunction->setWorkingBlock(b);
        }
        case StmtKind::Block: {
            scope.enterScope();

            for_reversed(ast->block->items) {
                if ((*it)->kind == BlockItemKind::Stmt) {
                    visit((*it)->stmt);
                } else {
                    visit((*it)->decl);
                }
            }

            scope.exitScope();
            return;// scope.currentFunction->setWorkingBlock(workingBlock);
        }
        case StmtKind::If: {
            auto cond = visit(ast->exp);
            auto elseBlockName = scope.currentFunction->assignNamedSymbol("%if_else");
            auto ifBlockName = scope.currentFunction->assignNamedSymbol("%if_then");
            auto outBlockName = scope.currentFunction->assignNamedSymbol("%if_out");

            if (ast->elseStmt) {

                WORKING_BLOCK->commitBlockByBranch((cond), ifBlockName, elseBlockName);

                auto &ifBlock = scope.currentFunction->createNewBlock(ifBlockName);
                scope.enterScope();
                WORKING_BLOCK = &ifBlock;
                visit(ast->ifStmt);
                WORKING_BLOCK->commitBlockByJump(outBlockName);
                scope.exitScope();

                auto &elseBlock = scope.currentFunction->createNewBlock(elseBlockName);
                scope.enterScope();
                WORKING_BLOCK = &elseBlock;
                visit(ast->elseStmt);
                WORKING_BLOCK->commitBlockByJump(outBlockName);
                scope.exitScope();

                auto &outBlock = scope.currentFunction->createNewBlock(outBlockName);
                return scope.currentFunction->setWorkingBlock(outBlock);

            } else {
                WORKING_BLOCK->commitBlockByBranch((cond), ifBlockName, outBlockName);

                auto &ifBlock = scope.currentFunction->createNewBlock(ifBlockName);
                scope.enterScope();
                WORKING_BLOCK = &ifBlock;
                visit(ast->ifStmt);
                WORKING_BLOCK->commitBlockByJump(outBlockName);
                scope.exitScope();

                auto &outBlock = scope.currentFunction->createNewBlock(outBlockName);
                return scope.currentFunction->setWorkingBlock(outBlock);
            }
        }
        case StmtKind::While: {
            auto routerBlockName = scope.currentFunction->assignNamedSymbol("%while_router");
            auto bodyBlockName = scope.currentFunction->assignNamedSymbol("%while_body");
            auto outBlockName = scope.currentFunction->assignNamedSymbol("%while_out");


            WORKING_BLOCK->commitBlockByJump(routerBlockName);


            auto &routerBlock = scope.currentFunction->createNewBlock(routerBlockName);

            WORKING_BLOCK = &routerBlock;
            auto cond = visit(ast->exp);
            WORKING_BLOCK->commitBlockByBranch((cond), bodyBlockName, outBlockName);


            auto &bodyBlock = scope.currentFunction->createNewBlock(bodyBlockName);
            scope.enterScope();
            scope.enterLoop(routerBlockName, outBlockName);
            WORKING_BLOCK = &bodyBlock;
            visit(ast->whileStmt);
            WORKING_BLOCK->commitBlockByJump(routerBlockName);
            scope.exitLoop();
            scope.exitScope();

            auto &outBlock = scope.currentFunction->createNewBlock(outBlockName);
            return scope.currentFunction->setWorkingBlock(outBlock);
        }
        case StmtKind::Break: {
            auto &loopInfo = scope.getLoopInfo();
            WORKING_BLOCK->commitBlockByJump(loopInfo.second);
            auto outBlockName = scope.currentFunction->assignNamedSymbol("%while_break_out");
            auto &outBlock = scope.currentFunction->createNewBlock(outBlockName);
            outBlock.isolated = true;
            return scope.currentFunction->setWorkingBlock(outBlock);
        }
        case StmtKind::Continue: {
            auto &loopInfo = scope.getLoopInfo();
            WORKING_BLOCK->commitBlockByJump(loopInfo.first);
            auto outBlockName = scope.currentFunction->assignNamedSymbol("%while_continue_out");
            auto &outBlock = scope.currentFunction->createNewBlock(outBlockName);
            outBlock.isolated = true;
            return scope.currentFunction->setWorkingBlock(outBlock);
        }
    }
    throw runtime_error("Branch not implemented.");
}


Operand Program::visit(const unique_ptr<ExpAST> &ast) {
    return visit(ast->lOrExp);
}

Operand Program::visit(const unique_ptr<LOrExpAST> &ast) {
    if (ast->kind == LOrExpKind::LAnd) {
        return visit(ast->lAnd);
    } else {

        auto shortHanded = scope.currentFunction->assignAnonymousSymbol();
        scope.currentFunction->createAlloc(shortHanded, Type{TypeKind::Int});

        auto assigned = scope.currentFunction->assignAnonymousSymbol();
        auto rhsBlockName = scope.currentFunction->assignNamedSymbol("%lor_rhs");
        auto outBlockName = scope.currentFunction->assignNamedSymbol("%lor_out");

        auto lhs = visit(ast->lOr);
        auto lhsBoolean = scope.currentFunction->assignAnonymousSymbol();
        WORKING_BLOCK->pushInstruction(IrKind::NotEq, lhsBoolean, lhs, 0);
        WORKING_BLOCK->pushInstruction(IrKind::Store, lhsBoolean, shortHanded);
        WORKING_BLOCK->commitBlockByBranch(lhsBoolean, outBlockName, rhsBlockName);


        auto &rhsBlock = scope.currentFunction->createNewBlock(rhsBlockName);
        scope.currentFunction->setWorkingBlock(rhsBlock);
        auto rhs = visit(ast->lAnd);
        auto rhsBoolean = scope.currentFunction->assignAnonymousSymbol();
        WORKING_BLOCK->pushInstruction(IrKind::NotEq, rhsBoolean, rhs, 0);
        WORKING_BLOCK->pushInstruction(IrKind::Store, rhsBoolean, shortHanded);
        WORKING_BLOCK->commitBlockByJump(outBlockName);

        auto &outBlock = scope.currentFunction->createNewBlock(outBlockName);
        scope.currentFunction->setWorkingBlock(outBlock);
        WORKING_BLOCK->pushInstruction(IrKind::Load, assigned, shortHanded);

        return assigned;
    }
}

Operand Program::visit(const unique_ptr<LAndExpAST> &ast) {
    if (ast->kind == LAndExpKind::Eq) {
        return visit(ast->eq);
    } else {
        auto shortHanded = scope.currentFunction->assignAnonymousSymbol();
        scope.currentFunction->createAlloc(shortHanded, Type{TypeKind::Int});
        auto assigned = scope.currentFunction->assignAnonymousSymbol();
        auto rhsBlockName = scope.currentFunction->assignNamedSymbol("%land_rhs");
        auto outBlockName = scope.currentFunction->assignNamedSymbol("%land_out");

        auto lhs = visit(ast->lAnd);
        auto lhsBoolean = scope.currentFunction->assignAnonymousSymbol();
        WORKING_BLOCK->pushInstruction(IrKind::NotEq, lhsBoolean, lhs, 0);
        WORKING_BLOCK->pushInstruction(IrKind::Store, lhsBoolean, shortHanded);
        WORKING_BLOCK->commitBlockByBranch(lhsBoolean, rhsBlockName, outBlockName);

        auto &rhsBlock = scope.currentFunction->createNewBlock(rhsBlockName);
        scope.currentFunction->setWorkingBlock(rhsBlock);
        auto rhs = visit(ast->eq);
        auto rhsBoolean = scope.currentFunction->assignAnonymousSymbol();
        WORKING_BLOCK->pushInstruction(IrKind::NotEq, rhsBoolean, rhs, 0);
        WORKING_BLOCK->pushInstruction(IrKind::Store, rhsBoolean, shortHanded);
        WORKING_BLOCK->commitBlockByJump(outBlockName);

        auto &outBlock = scope.currentFunction->createNewBlock(outBlockName);
        scope.currentFunction->setWorkingBlock(outBlock);
        WORKING_BLOCK->pushInstruction(IrKind::Load, assigned, shortHanded);

        return assigned;
    }
}

Operand Program::visit(const unique_ptr<EqExpAST> &ast) {
    if (ast->kind == EqExpKind::Rel) {
        return visit(ast->rel);
    } else {
        auto lhs = visit(ast->eq);
        auto rhs = visit(ast->rel);
        auto assigned = scope.currentFunction->assignAnonymousSymbol();
        auto op = ast->op == "!=" ? IrKind::NotEq : IrKind::Eq;
        WORKING_BLOCK->pushInstruction(op, assigned, lhs, rhs);
        return assigned;
    }
}

Operand Program::visit(const unique_ptr<RelExpAST> &ast) {
    if (ast->kind == RelExpKind::Add) {
        return visit(ast->add);
    } else {
        auto lhs = visit(ast->rel);
        auto rhs = visit(ast->add);
        auto assigned = scope.currentFunction->assignAnonymousSymbol();
        auto op = ast->op == "<" ? IrKind::Lt
                                 : ast->op == ">" ? IrKind::Gt
                                                  : ast->op == "<=" ? IrKind::Le
                                                                    : IrKind::Ge;

        WORKING_BLOCK->pushInstruction(op, assigned, lhs, rhs);
        return assigned;
    }
}

Operand Program::visit(const unique_ptr<AddExpAST> &ast) {
    if (ast->kind == AddExpKind::Mul) {
        return visit(ast->mul);
    } else {
        auto lhs = visit(ast->add);
        auto rhs = visit(ast->mul);
        auto assigned = scope.currentFunction->assignAnonymousSymbol();
        auto op = ast->op == '+' ? IrKind::Add : IrKind::Sub;
        WORKING_BLOCK->pushInstruction(op, assigned, lhs, rhs);
        return assigned;
    }
}

Operand Program::visit(const unique_ptr<MulExpAST> &ast) {
    if (ast->kind == MulExpKind::Unary) {
        return visit(ast->unary);
    } else {
        auto lhs = visit(ast->mul);
        auto rhs = visit(ast->unary);
        auto assigned = scope.currentFunction->assignAnonymousSymbol();
        auto op = ast->op == '*' ? IrKind::Mul
                                 : ast->op == '/' ? IrKind::Div
                                                  : IrKind::Mod;
        WORKING_BLOCK->pushInstruction(op, assigned, lhs, rhs);
        return assigned;
    }
}


Operand Program::visit(const unique_ptr<UnaryExpAST> &ast) {
    switch (ast->kind) {
        case UnaryExpKind::Primary: {
            return visit(ast->primary);
        }
        case UnaryExpKind::Call: {
            const auto &funRecord = scope.getFunctionSignature(ast->ident);
            auto rParams = vector<Operand>();
            for_reversed(ast->funcRParams) {
                rParams.emplace_back(visit(*it));
            }
            const uint paramSize = funRecord.params.size();
            for (uint i = 0; i < paramSize; i++) {
                if (funRecord.params[i]->kind == TypeKind::Pointer) {
                    auto ptr = scope.currentFunction->assignAnonymousSymbol();
                    WORKING_BLOCK->pushInstruction(
                        IrKind::GetElemPtr,
                        ptr,
                        rParams[i],
                        0
                    );
                    rParams[i] = ptr;
                }
            }

            if (funRecord.returnType->kind == TypeKind::Int) {
                auto assigned = scope.currentFunction->assignAnonymousSymbol();
                WORKING_BLOCK->pushCallInstruction(
                    "@" + ast->ident,
                    assigned,
                    rParams
                );
                return assigned;
            } else {
                WORKING_BLOCK->pushCallInstruction(
                    "@" + ast->ident,
                    rParams
                );
                return {""};
            }
        }
        case UnaryExpKind::Unary: {
            auto rVal = visit(ast->unary);
            if (ast->unaryOp == '+') {
                return rVal;
            } else if (ast->unaryOp == '!') {
                auto assigned = scope.currentFunction->assignAnonymousSymbol();
                WORKING_BLOCK->pushInstruction(IrKind::Eq, assigned, rVal, 0);
                return assigned;
            } else {
                auto assigned = scope.currentFunction->assignAnonymousSymbol();
                WORKING_BLOCK->pushInstruction(IrKind::Sub, assigned, 0, rVal);
                return assigned;
            }
        }
    }
}


Operand Program::visit(const unique_ptr<PrimaryExpAST> &ast) {
    switch (ast->kind) {
        case PrimaryExpKind::Number : {
            return ast->number;
        }
        case PrimaryExpKind::Exp : {
            return visit(ast->exp);
        }
        case PrimaryExpKind::LVal : {
            const auto &val = scope.getValue(ast->lVal->ident);
            if (ast->lVal->dimExps.empty()) {
                if (val.kind == ValueKind::Const) {
                    return (val.constData);
                } else if (val.type->kind == TypeKind::Int) {
                    auto assigned = scope.currentFunction->assignAnonymousSymbol();
                    WORKING_BLOCK->pushInstruction(IrKind::Load, assigned, val.valueData);
                    return assigned;
                } else {
                    return val.valueData;
                }
            } else {
                Operand ptr = val.valueData;
                Type *currType = val.type.get();

                for_reversed(ast->lVal->dimExps) {
                    auto assigned = scope.currentFunction->assignAnonymousSymbol();
                    if (currType->kind == TypeKind::Pointer) {
                        auto container = scope.currentFunction->assignAnonymousSymbol();
                        WORKING_BLOCK->pushInstruction(
                            IrKind::Load,
                            container,
                            ptr
                        );
                        WORKING_BLOCK->pushInstruction(
                            IrKind::GetPtr,
                            assigned,
                            container,
                            visit(*it)
                        );
                    } else if (currType->kind == TypeKind::Array) {
                        WORKING_BLOCK->pushInstruction(
                            IrKind::GetElemPtr,
                            assigned,
                            ptr,
                            visit(*it)
                        );
                    }
                    ptr = assigned;
                    currType = currType->baseType.get();
                }
                if (currType->kind == TypeKind::Int) {
                    auto assigned = scope.currentFunction->assignAnonymousSymbol();
                    WORKING_BLOCK->pushInstruction(IrKind::Load, assigned, ptr);
                    return assigned;
                }
                return ptr;
            }
        }
    }
}

#undef for_reversed

ostream &ir::operator<<(ostream &os, const Program &prog) {
    for (const auto &item: prog.scope.getGlobals()) {
        switch (item.index()) {
            case 0:
                os << *get<unique_ptr<Function>>(item) << endl;
                break;
            case 1:
                os << *get<unique_ptr<GlobalValue>>(item) << endl;
                break;
            case 2:
                os << *get<unique_ptr<GlobalDecl>>(item) << endl;
                break;
        }
    }
    return os;
}