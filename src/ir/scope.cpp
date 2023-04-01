//
// Created by 李臻 on 2022/5/9.
//

#include "scope.h"

#include <memory>

using namespace ir;


#define for_reversed(items) for(auto it = items.rbegin(); it != items.rend(); it++)
#define WORKING_BLOCK currentFunction->workingBlock


optional<int> Scope::eval(ExpAST &ast) {
    return eval(*ast.lOrExp);
}


optional<int> Scope::eval(EqExpAST &ast) {
    if (ast.kind == EqExpKind::Rel) {
        return eval(*ast.rel);
    } else {
        auto lhs = eval(*ast.eq);
        auto &op = ast.op;
        auto rhs = eval(*ast.rel);
        if (lhs && rhs) {
            if (op == "==") {
                return lhs.value() == rhs.value();
            } else {
                return lhs.value() != rhs.value();
            }
        } else {
            return nullopt;
        }
    }
}

optional<int> Scope::eval(LValAST &ast) {
    auto &val = getValue(ast.ident);
    if (ast.dimExps.empty()) {
        if (val.kind == ValueKind::Const) {
            return val.constData;
        } else {
            return nullopt;
        }
    }
    return nullopt;
}

optional<int> Scope::eval(PrimaryExpAST &ast) {
    switch (ast.kind) {
        case PrimaryExpKind::Exp: {
            return eval(*ast.exp);
        }
        case PrimaryExpKind::LVal: {
            return eval(*ast.lVal);
        }
        case PrimaryExpKind::Number: {
            return ast.number;
        }
    }
    return nullopt;
}

optional<int> Scope::eval(UnaryExpAST &ast) {
    switch (ast.kind) {
        case UnaryExpKind::Primary: {
            return eval(*ast.primary);
        }
        case UnaryExpKind::Call: {
            return nullopt;
        }
        case UnaryExpKind::Unary: {
            auto result = eval(*ast.unary);
            if (!result.has_value()) {
                return nullopt;
            } else if (ast.unaryOp == '-') {
                return -result.value();
            } else if (ast.unaryOp == '!') {
                return (result == 0);
            }
        }
    }
    return nullopt;
}


optional<int> Scope::eval(MulExpAST &ast) {
    if (ast.kind == MulExpKind::Unary) {
        return eval(*ast.unary);
    } else {
        auto lhs = eval(*ast.mul);
        auto &op = ast.op;
        auto rhs = eval(*ast.unary);
        if (lhs && rhs) {
            if (op == '*') {
                return lhs.value() * rhs.value();
            } else if (op == '/' && rhs.value() != 0) {
                return lhs.value() / rhs.value();
            } else if (op == '%' && rhs.value() != 0) {
                return lhs.value() % rhs.value();
            } else return nullopt;
        } else {
            return nullopt;
        }
    }
}


optional<int> Scope::eval(AddExpAST &ast) {
    if (ast.kind == AddExpKind::Mul) {
        return eval(*ast.mul);
    } else {
        auto lhs = eval(*ast.add);
        auto &op = ast.op;
        auto rhs = eval(*ast.mul);
        if (lhs && rhs) {
            if (op == '+') {
                return lhs.value() + rhs.value();
            } else if (op == '-') {
                return lhs.value() - rhs.value();
            }
        } else {
            return nullopt;
        }
    }
    return nullopt;
}

optional<int> Scope::eval(RelExpAST &ast) {
    if (ast.kind == RelExpKind::Add) {
        return eval(*ast.add);
    } else {
        auto lhs = eval(*ast.rel);
        auto &op = ast.op;
        auto rhs = eval(*ast.add);
        if (lhs && rhs) {
            if (op == "<") {
                return lhs.value() < rhs.value();
            } else if (op == ">") {
                return lhs.value() > rhs.value();
            } else if (op == "<=") {
                return lhs.value() <= rhs.value();
            } else if (op == ">=") {
                return lhs.value() >= rhs.value();
            }
        } else {
            return nullopt;
        }
    }
    return nullopt;
}


optional<int> Scope::eval(LAndExpAST &ast) {
    if (ast.kind == LAndExpKind::Eq) {
        return eval(*ast.eq);
    } else {
        auto lhs = eval(*ast.lAnd);
        auto rhs = eval(*ast.eq);
        if (lhs && rhs) {
            return (lhs.value() != 0 && rhs.value() != 0);
        } else {
            return nullopt;
        }
    }
}


optional<int> Scope::eval(LOrExpAST &ast) {
    if (ast.kind == LOrExpKind::LAnd) {
        return eval(*ast.lAnd);
    } else {
        auto lhs = eval(*ast.lOr);
        auto rhs = eval(*ast.lAnd);
        if (lhs && rhs) {
            return (lhs.value() != 0 || rhs.value() != 0);
        } else {
            return nullopt;
        }
    }
}

optional<int> Scope::eval(ConstExpAST &ast) {
    return eval(*ast.exp);
}

Scope::Scope() {
    valueMaps = vector<unordered_map<string, Value>>{
        unordered_map<string, Value>()
    };
    functionSignatures = unordered_map<string, unique_ptr<FunctionSignature>>();
    loopInfo = vector<pair<const string, const string>>();
}


bool Scope::isGlobal() const {
    return !currentFunction;
}

const Value &Scope::getValue(string &ident) const {
    for (auto valueMap = valueMaps.rbegin();
         valueMap != valueMaps.rend();
         valueMap++) {
        auto &valueMapRef = *valueMap;
        auto it = valueMap->find(ident);
        if (it != valueMapRef.end()) {
            return it->second;
        }
    }
    throw runtime_error("Value symbol not found. ");
}

const FunctionSignature &Scope::getFunctionSignature(string &ident) const {
    auto it = functionSignatures.find(ident);
    if (it != functionSignatures.end()) {
        return *(it->second);
    }
    throw runtime_error("Function symbol not found.");
}

void Scope::addValue(const string &ident, Value &value) {
    bool isGlobal = this->isGlobal();
    auto &currentMap = *valueMaps.rbegin();

    bool isDuplicate = (currentMap.find(ident) != currentMap.end())
                       || (isGlobal && functionSignatures.find(ident) != functionSignatures.end());

    if (isDuplicate) {
        throw runtime_error("Duplicate definition.");
    } else {
        string copiedItem = string(ident);
        currentMap.emplace(copiedItem, value);
    }
}

void Scope::addFunctionSignature(const string &ident, unique_ptr<FunctionSignature> functionSignature) {
    bool isDuplicate = (functionSignatures.find(ident) != functionSignatures.end()) ||
                       (!valueMaps.empty() && (valueMaps.begin()->find(ident) != valueMaps.begin()->end()));
    if (isDuplicate) {
        throw runtime_error("Duplicate function definition.");
    } else {
        string copiedIdent = string(ident);
        functionSignatures.emplace(copiedIdent, move(functionSignature));
    }
}

void Scope::enterScope() {
    valueMaps.emplace_back();
}

void Scope::exitScope() {
    valueMaps.pop_back();
}

/**
 * Enter function, alloc in params
 * @param header
 * @param params
 */
void Scope::enterFunction(const FuncHeaderAST &header, const vector<unique_ptr<FuncFParamAST>> &params) {
    // already in scope

    currentFunction = make_unique<Function>(header);

    vector<pair<string, unique_ptr<Type>>> paramTypes = vector<pair<string, unique_ptr<Type>>>();

    for (const auto &it: globals) {
        switch (it.index()) {
            case 0: {
                // no @
                const string symbolName = "@" + get<unique_ptr<Function>>(it)->ident;
                currentFunction->assignNamedSymbol(symbolName);
                break;
            }
            case 1: {
                const string symbolName = get<unique_ptr<GlobalValue>>(it)->ident;
                currentFunction->assignNamedSymbol(symbolName);
                break;
            }
            case 2:
                const string symbolName = get<unique_ptr<GlobalDecl>>(it)->ident;
                currentFunction->assignNamedSymbol(symbolName);
                break;
        }

    }

    for_reversed(params) {
        auto &param = **it;
//        param.
        auto paramInputName = currentFunction->assignAnonymousSymbol();
        auto paramHolderName = currentFunction->assignNamedSymbol("@" + param.ident);

        auto &paramType = paramTypes.emplace_back(
            paramInputName,
            getTypeFromDims(param.dimConstExps)
        );
        currentFunction->createAlloc(paramHolderName, *paramType.second);
        currentFunction->workingBlock->pushInstruction(
            IrKind::Store,
            paramInputName,
            paramHolderName
        );

        auto value = Value{paramHolderName, paramType.second};
        addValue(param.ident, value);
    }
    currentFunction->setParamTypes(paramTypes);
    addFunctionSignature(header.ident, currentFunction->getSignature());

    // TODO: assign
//    for (const auto &it: functionSignatures) {
//        currentFunction->assignNamedSymbol(it.first);
//    }
}


void Scope::exitFunction() {
    captureGlobals(currentFunction);
//    globalItems.push_back(move(currentFunction));
    currentFunction = nullptr;
}


void Scope::enterLoop(const string &router, const string &out) {
    loopInfo.emplace_back(router, out);
}

void Scope::exitLoop() {
    loopInfo.pop_back();
}

pair<const string, const string> &Scope::getLoopInfo() {
    return *loopInfo.rbegin();
}

/**
 * Used for funcFParamsAST
 * @param dims
 * @return
 */
unique_ptr<Type> Scope::getTypeFromDims(const optional<vector<unique_ptr<ConstExpAST>>> &dims) {
    if (dims) {
        auto original = Type::getInt();
        for_reversed(dims.value()) {
            original = Type::getArrayFor(
                move(original),
                eval(*((*it)->exp)).value()
            );
        }
        return Type::getPointerFor(move(original));
    } else {
        return Type::getInt();
    }
}

void Scope::initializeStdlib() {
//    return;

    const auto getIntSignature = new FunctionSignature{
        Type{
            TypeKind::Int
        }
    };
    auto getIntDecl = make_unique<GlobalDecl>(
        "@getint",
        *getIntSignature
    );
    captureGlobals(getIntDecl);
    functionSignatures.emplace("getint", getIntSignature);


    const auto getChSignature = new FunctionSignature{
        Type{
            TypeKind::Int
        }
    };
    auto getChDecl = make_unique<GlobalDecl>("@getch", *getChSignature);
    captureGlobals(getChDecl);
    functionSignatures.emplace("getch", getChSignature);


    const auto getArraySignature = new FunctionSignature{
        Type{
            TypeKind::Int,
        },
        Type{
            TypeKind::Pointer,
            make_unique<Type>(
                TypeKind::Int
            )
        }
    };
    auto getArrayDecl = make_unique<GlobalDecl>("@getarray", *getArraySignature);
    captureGlobals(getArrayDecl);
    functionSignatures.emplace("getarray", getArraySignature);


    auto putIntSignature = new FunctionSignature{
        vector<unique_ptr<Type>>(),
        Type{TypeKind::Unit}
    };
    putIntSignature->params.emplace_back(new Type{TypeKind::Int});
    auto putIntDecl = make_unique<GlobalDecl>("@putint", *putIntSignature);
    captureGlobals(putIntDecl);
    functionSignatures.emplace(
        "putint",
        putIntSignature
    );


    auto putChSignature = new FunctionSignature{
        vector<unique_ptr<Type>>(),
        Type{TypeKind::Unit}
    };
    putChSignature->params.emplace_back(new Type{TypeKind::Int});
    functionSignatures.emplace(
        "putch",
        putChSignature
    );
    auto putChDecl = make_unique<GlobalDecl>("@putch", *putChSignature);
    captureGlobals(putChDecl);

    auto putArraySignature = new FunctionSignature{
        vector<unique_ptr<Type>>(),
        Type{TypeKind::Unit}
    };
    putArraySignature->params.emplace_back(new Type{TypeKind::Int});
    putArraySignature->params.emplace_back(
        new Type{
            TypeKind::Pointer,
            make_unique<Type>(TypeKind::Int)
        }
    );
    auto putArrayDecl = make_unique<GlobalDecl>("@putarray", *putArraySignature);
    captureGlobals(putArrayDecl);
    functionSignatures.emplace(
        "putarray",
        putArraySignature
    );


    const auto startTimeSignature = new FunctionSignature{
        vector<unique_ptr<Type>>(),
        Type{TypeKind::Unit
        }
    };
    auto startTimeDecl = make_unique<GlobalDecl>("@starttime", *startTimeSignature);
    captureGlobals(startTimeDecl);
    functionSignatures.emplace(
        "starttime",
        startTimeSignature
    );


    const auto stopTimeSignature = new FunctionSignature{
        vector<unique_ptr<Type>>(),
        Type{TypeKind::Unit
        }
    };
    auto stopTimeDecl = make_unique<GlobalDecl>("@stoptime", *stopTimeSignature);
    captureGlobals(stopTimeDecl);
    functionSignatures.emplace(
        "stoptime",
        stopTimeSignature
    );
}

void Scope::captureGlobals(unique_ptr<Function> &func) {
    globals.emplace_back(move(func));
};

void Scope::captureGlobals(unique_ptr<GlobalValue> &val) {
    globals.emplace_back(move(val));
};

void Scope::captureGlobals(unique_ptr<GlobalDecl> &decl) {
    globals.emplace_back(move(decl));
};


const vector<GlobalItem> &Scope::getGlobals() const {
    return globals;
}


void Scope::initArrayOnBlock(const string &symbol, const Type &type, const Initializer &init) {
    assert(currentFunction);
    switch (type.kind) {
        case TypeKind::Int: {
            assert(init.kind != InitializerKind::List);
//            const string entryName = currentFunction->assignAnonymousSymbol();
            if (init.kind == InitializerKind::Value) {
                currentFunction->workingBlock->pushInstruction(
                    IrKind::Store,
                    init.valueData,
                    symbol
                );
            } else {
                currentFunction->workingBlock->pushInstruction(
                    IrKind::Store,
                    init.constData,
                    symbol
                );
            }
            return;
        }
        case TypeKind::Array: {
            assert(init.kind == InitializerKind::List && init.listData.size() == type.arraySize);
            const uint dimSize = init.listData.size();
            for (uint i = 0; i < dimSize; i++) {
                const string entryName = currentFunction->assignNamedSymbol("%entry");
                currentFunction->workingBlock->pushInstruction(
                    IrKind::GetElemPtr,
                    entryName,
                    symbol,
                    (int) i
                );
                initArrayOnBlock(entryName, *type.baseType, init.listData[i]);
            }
            return;
        }
        default: {
            throw runtime_error("Unexpected type.");
        }
    }
}