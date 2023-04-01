//
// Created by 李臻 on 2022/5/22.
//

#include "program.h"

using namespace risk;

#define with(slice) for (size_t i = 0; i < slice.len; ++i)

bool Program::isGlobal(KValue const &val) const {
    return values.find(val) != values.end();
}

void Program::setValue(const string &name, const KValue &val) {
    values.emplace(val, name);
}

void Program::visit(const KProgram &program, ostream &os) {
    with(program.values) {
        const KValue &globalVal = reinterpret_cast<KValue>(program.values.buffer[i]);
        const string globalValName = dropFirst(globalVal->name);
        setValue(globalValName, globalVal);
        os << "  .data" << endl
           << "  .globl " << globalValName << endl
           << globalValName << ":" << endl;
        visit(*globalVal, os);
        os << endl;
    }

    with(program.funcs) {
        const KFunction &fun = reinterpret_cast<KFunction>(program.funcs.buffer[i]);
        visit(fun, os);
    }
}

void Program::visit(const KFunction &fun, ostream &os) {
    if (fun->bbs.len == 0) return;
    currentFunction = make_unique<Function>(fun);
    for (const auto &item: currentFunction->getAllKValueDatum()) {
        cerr << (item->kind.tag) << endl;
        if (isLocal(item->kind.tag) && item->used_by.len > 0) {
            currentFunction->allocSlot(*item);
        }
        if (item->kind.tag == KOOPA_RVT_CALL) {
            currentFunction->captureArgNum(item->kind.data.call.args.len);
        }
    }

    for (const auto &item: currentFunction->gatAllKBasicBlocks()) {
        currentFunction->captureBlockName(item, item->name);
    }

    auto funName = dropFirst(fun->name);
    os << "  .text" << endl
       << "  .globl " << funName << endl
       << funName << ": " << endl;
    auto asmStream = AsmStream(os);
    auto offset = (int) currentFunction->getSpOffset();
    if (offset != 0) {
        asmStream.addi("sp", "sp", -offset);
        if (currentFunction->maxArgNum.has_value())
            asmStream.sw("ra", "sp", offset - 4);
    }

    with(fun->bbs) {
        const auto bb = reinterpret_cast<KBasicBlock>(fun->bbs.buffer[i]);
        const auto blockName = visit(bb, os);
        os << blockName << ":\n";
        for (int j = 0; j < bb->insts.len; j++) {
            visit(*reinterpret_cast<KValue>(bb->insts.buffer[j]), os);
        }
    }
    currentFunction.release();
}

void Program::visit(const KGlobalAlloc &global, ostream &os) {
    os << "  .data" << endl;
}

//string Program::visit(const KFunction &fun, ostream &os) {
//    return dropFirst(fun->name);
//}

Record Program::visit(const KValue &val, ostream &os) {
    if (isGlobal(val)) return {values[val]};
    if (val->kind.tag == KOOPA_RVT_INTEGER) return {(int) val->kind.data.integer.value};
    if (val->kind.tag == KOOPA_RVT_FUNC_ARG_REF)return {(unsigned int) val->kind.data.func_arg_ref.index};
    return Record::makeRecord(currentFunction->getSlotOffset(*val));
}


void Program::visit(const KValueData &val, ostream &os) {
    const auto &kind = val.kind;
    switch (kind.tag) {
        case KOOPA_RVT_INTEGER: {
            os << "  .word " << val.kind.data.integer.value << endl;
            break;
        }
        case KOOPA_RVT_ZERO_INIT: {
            os << "  .zero " << SizeOf(val.ty) << endl;
            break;
        }
        case KOOPA_RVT_AGGREGATE: {
            auto &elems = val.kind.data.aggregate.elems;
            with(elems) {
                visit(*reinterpret_cast<KValue>(elems.buffer[i]), os);
            }
            break;
        }
        case KOOPA_RVT_UNDEF:
        case KOOPA_RVT_FUNC_ARG_REF:   /// Function argument reference.
        case KOOPA_RVT_BLOCK_ARG_REF: /// Basic block argument reference.
        case KOOPA_RVT_ALLOC: {
            break;
        }
        case KOOPA_RVT_GLOBAL_ALLOC: {
            const KValueData &init = *kind.data.global_alloc.init;
            visit(init, os);
            break;
        }
        case KOOPA_RVT_LOAD: {
            auto src = visit(val.kind.data.load.src, os);
            src.write(os, "t0");
            if (src.index() == Record::Local && src.asLocal().isPointer) {
                AsmStream(os, "t1").lw("t0", "t0", 0);
            }
            auto offset = currentFunction->getSlotOffset(val);
            Record::makeRecord(offset).read(os, "t0", "t1");
            break;
        }
        case KOOPA_RVT_STORE: {
            auto spOffset = currentFunction->getSpOffset();
            auto value = visit(val.kind.data.store.value, os);
            if (value.index() == Record::Arg) {
                value.writeSp(os, "t0", spOffset);
            } else {
                value.write(os, "t0");
            }
            auto dst = visit(val.kind.data.store.dest, os);
            if (dst.index() == Record::Local && dst.asLocal().isPointer) {
                dst.write(os, "t1");
                AsmStream(os, "t2").sw("t0", "t1", 0);
            } else {
                dst.read(os, "t0", "t1");
            }
            break;
        }
        case KOOPA_RVT_GET_PTR: {
            auto src = visit(kind.data.get_ptr.src, os);
            if (src.index() == Record::Local && src.asLocal().isPointer) {
                src.write(os, "t0");
            } else {
                src.writeAddr(os, "t0");
            }
            auto index = visit(kind.data.get_ptr.index, os);
            index.write(os, "t1");
            const auto sz = SizeOf(val.ty->data.pointer.base);
            AsmStream(os, "t2").muli("t1", "t1", sz)
                .op("add ", "t0", "t0", "t1");
            Record::makeRecord(currentFunction->getSlotOffset(val)).read(os, "t0", "t1");
            break;
        }
        case KOOPA_RVT_GET_ELEM_PTR: {
            auto src = visit(kind.data.get_elem_ptr.src, os);
            if (src.index() == Record::Local && src.asLocal().isPointer) {
                src.write(os, "t0");
            } else {
                src.writeAddr(os, "t0");
            }
            auto index = visit(kind.data.get_elem_ptr.index, os);
            index.write(os, "t1");
            const auto sz = SizeOf(val.ty->data.pointer.base);
            AsmStream(os, "t2").muli("t1", "t1", (int) sz)
                .op("add ", "t0", "t0", "t1");
            Record::makeRecord(currentFunction->getSlotOffset(val)).read(os, "t0", "t1");
            break;
        }
        case KOOPA_RVT_BINARY: {
            auto lhsVal = visit(kind.data.binary.lhs, os);
            auto rhsVal = visit(kind.data.binary.rhs, os);
            lhsVal.write(os, "t0");
            rhsVal.write(os, "t1");

            AsmStream inst = AsmStream(os, "t2");
            auto op = static_cast<KBinaryOp>(kind.data.binary.op);
            switch (op) {
                case KBinaryOp::NotEq: {
                    inst.op("xor ", "t0", "t0", "t1")
                        .op("snez", "t0", "t0");
                    break;
                }
                case KBinaryOp::Eq: {
                    inst.op("xor ", "t0", "t0", "t1")
                        .op("seqz", "t0", "t0");
                    break;
                }
                case KBinaryOp::Gt: {
                    inst.op("sgt ", "t0", "t0", "t1");
                    break;
                }
                case KBinaryOp::Lt: {
                    inst.op("slt ", "t0", "t0", "t1");
                    break;
                }
                case KBinaryOp::Ge: {
                    inst.op("slt ", "t0", "t0", "t1")
                        .op("seqz", "t0", "t0");
                    break;
                }
                case KBinaryOp::Le: {
                    inst.op("sgt ", "t0", "t0", "t1")
                        .op("seqz", "t0", "t0");
                    break;
                }
                case KBinaryOp::Add: {
                    inst.op("add ", "t0", "t0", "t1");
                    break;
                }
                case KBinaryOp::Sub: {
                    inst.op("sub ", "t0", "t0", "t1");
                    break;
                }
                case KBinaryOp::Mul: {
                    inst.op("mul ", "t0", "t0", "t1");
                    break;
                }
                case KBinaryOp::Div: {
                    inst.op("div ", "t0", "t0", "t1");
                    break;
                }
                case KBinaryOp::Mod: {
                    inst.op("rem ", "t0", "t0", "t1");
                    break;
                }
                case KBinaryOp::And: {
                    inst.op("and ", "t0", "t0", "t1");
                    break;
                }
                case KBinaryOp::Or: {
                    inst.op("or  ", "t0", "t0", "t1");
                    break;
                }
                case KBinaryOp::Xor: {
                    inst.op("xor ", "t0", "t0", "t1");
                    break;
                }
                case KBinaryOp::Shl: {
                    inst.op("shl ", "t0", "t0", "t1");
                    break;
                }
                case KBinaryOp::Shr: {
                    inst.op("shr ", "t0", "t0", "t1");
                    break;
                }
                case KBinaryOp::Sar: {
                    inst.op("sar ", "t0", "t0", "t1");
                    break;
                }
            }
            Record::makeRecord(currentFunction->getSlotOffset(val)).read(os, "t0", "t1");
            break;
        }
        case KOOPA_RVT_BRANCH: {
            const KBranch &branch = kind.data.branch;
            visit(branch.cond, os).write(os, "t0");
            const auto label1 = visit(branch.true_bb, os);
            const auto label2 = visit(branch.false_bb, os);
            AsmStream(os, "t1").bnez("t0", label1).j(label2);
            break;
        }
        case KOOPA_RVT_JUMP: {
            cerr << "JUMP" << endl;
            const KJump &jump = kind.data.jump;
            auto label = visit(jump.target, os);
            AsmStream(os).j(label);
            break;
        }
        case KOOPA_RVT_CALL: {
            const KCall &call = kind.data.call;
            auto argVals = vector<Record>();
            with(call.args) {
                argVals.push_back(
                    visit(reinterpret_cast<KValue>(call.args.buffer[i]), os)
                );
            }
            for (unsigned int i = 0; i < call.args.len; i++) {
                argVals[i].write(os, "t0");
                Record{i}.read(os, "t0", "t1");
            }
            const string callee = dropFirst(call.callee->name);
            AsmStream(os).call(callee);
            if (val.used_by.len > 0) {
                Record::makeRecord(currentFunction->getSlotOffset(val))
                    .read(os, "a0", "t0");
            }
            break;
        }
        case KOOPA_RVT_RETURN: {
            const KReturn &ret = kind.data.ret;
            if (ret.value != nullptr) {
                visit(ret.value, os).write(os, "a0");
            }

            auto offset = (int) currentFunction->getSpOffset();
            auto asmStream = AsmStream(os);
            if (offset != 0) {
                if (currentFunction->maxArgNum.has_value()) {
                    asmStream.lw("ra", "sp", offset - 4);
                }
                asmStream.addi("sp", "sp", offset);
            }
            os << "  ret" << endl << endl;
            break;
        }
        default: {
            assert(false);
        }
    }
}

string Program::visit(const KBasicBlock &block, ostream &os) {
    return currentFunction->getBlockName(block);
}

#undef with