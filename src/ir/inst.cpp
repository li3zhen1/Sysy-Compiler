//
// Created by 李臻 on 2022/5/17.
//

#include "inst.h"


ostream &ir::operator<<(ostream &os, const Instruction &inst) {
    os << "  ";
    switch (inst.kind) {
        case IrKind::Call: {
            if (inst.callRet) {
                os << inst.callRet.value() << " = ";
            }
            os << "call " << inst.callFun << "(";
            for (auto it = inst.callParams.begin();
                 it != inst.callParams.end();
                 it++) {
                if (it != inst.callParams.begin())
                    os << ", ";
                os << *it;
            }
            return os << ")\n";
        }
        case IrKind::Ret: {
            if (inst.ret) {
                return os << "ret " << inst.op1 << endl;
            }
            return os << "ret" << endl;
        }
        case IrKind::Alloc: {
            if (inst.allocType) {
                return os << inst.op1 << " = alloc " << *inst.allocType << endl;
            } else {
                throw runtime_error("Alloc type not specified");
            }
        }
        case IrKind::Br: {
            return os << "br " << inst.op1 << ", " << inst.op2 << ", " << inst.op3 << endl;
        }
        case IrKind::Jump: {
            return os << "jump " << inst.op1 << endl;
        }
        case IrKind::Load: {
            return os << inst.op1 << " = load " << inst.op2 << endl;
        }
        case IrKind::Store: {
            return os << "store " << inst.op1 << ", " << inst.op2 << endl;
        }

        case IrKind::GetElemPtr: {
            return os << inst.op1 << " = getelemptr " << inst.op2 << ", " << inst.op3 << endl;
        }
        case IrKind::GetPtr: {
            return os << inst.op1 << " = getptr " << inst.op2 << ", " << inst.op3 << endl;
        }


        case IrKind::NotEq: {
            return os << inst.op1 << " = ne " << inst.op2 << ", " << inst.op3 << endl;
        }
        case IrKind::Eq: {
            return os << inst.op1 << " = eq " << inst.op2 << ", " << inst.op3 << endl;
        }
        case IrKind::Gt: {
            return os << inst.op1 << " = gt " << inst.op2 << ", " << inst.op3 << endl;
        }
        case IrKind::Lt: {
            return os << inst.op1 << " = lt " << inst.op2 << ", " << inst.op3 << endl;
        }
        case IrKind::Ge: {
            return os << inst.op1 << " = ge " << inst.op2 << ", " << inst.op3 << endl;
        }
        case IrKind::Le: {
            return os << inst.op1 << " = le " << inst.op2 << ", " << inst.op3 << endl;
        }
        case IrKind::Add: {
            return os << inst.op1 << " = add " << inst.op2 << ", " << inst.op3 << endl;
        }
        case IrKind::Sub: {
            return os << inst.op1 << " = sub " << inst.op2 << ", " << inst.op3 << endl;
        }
        case IrKind::Mul: {
            return os << inst.op1 << " = mul " << inst.op2 << ", " << inst.op3 << endl;
        }
        case IrKind::Div: {
            return os << inst.op1 << " = div " << inst.op2 << ", " << inst.op3 << endl;
        }
        case IrKind::Mod: {
            return os << inst.op1 << " = mod " << inst.op2 << ", " << inst.op3 << endl;
        }
        case IrKind::And: {
            return os << inst.op1 << " = and " << inst.op2 << ", " << inst.op3 << endl;
        }
        case IrKind::Or: {
            return os << inst.op1 << " = or " << inst.op2 << ", " << inst.op3 << endl;
        }
        case IrKind::Xor: {
            return os << inst.op1 << " = xor " << inst.op2 << ", " << inst.op3 << endl;
        }
        case IrKind::Shl: {
            return os << inst.op1 << " = shl " << inst.op2 << ", " << inst.op3 << endl;
        }
        case IrKind::Shr: {
            return os << inst.op1 << " = shr " << inst.op2 << ", " << inst.op3 << endl;
        }
        case IrKind::Sar: {
            return os << inst.op1 << " = sar " << inst.op2 << ", " << inst.op3 << endl;
        }


    }
}

ir::Instruction::Instruction(const IrKind &_kind, const Operand &_op1) {
    kind = _kind;
    op1 = _op1;
    if (_kind == IrKind::Ret) {
        ret = _op1;
    }
}

ir::Instruction::Instruction(const IrKind &_kind, const Operand &_op1, const Operand &_op2) {
    kind = _kind;
    op1 = _op1;
    op2 = _op2;
}

ir::Instruction::Instruction(const IrKind &_kind, const Operand &_op1, const Operand &_op2, const Operand &_op3) {
    kind = _kind;
    op1 = _op1;
    op2 = _op2;
    op3 = _op3;
}

ir::Instruction::Instruction(const string &_fun, const string &_callRet, const vector<Operand> &_callParams) {
    callRet = _callRet;
    callFun = _fun;
    callParams = _callParams;
    kind = IrKind::Call;
}

ir::Instruction::Instruction(const string &_fun, const vector<Operand> &_callParams) {
    callFun = _fun;
    callParams = _callParams;
    kind = IrKind::Call;
}

ir::Instruction::Instruction(const ir::IrKind &_kind) {
    kind = _kind;
}

ir::Instruction::Instruction(const string &_symbolName, const ir::Type &_ty) {
    kind = IrKind::Alloc;
    op1 = _symbolName;
    allocType = _ty.clone();
}
