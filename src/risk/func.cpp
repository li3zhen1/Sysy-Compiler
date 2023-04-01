//
// Created by 李臻 on 2022/5/23.
//

#include "func.h"

using namespace risk;

Function::Function(const KFunction &_fun) :
    allocSize(0),
    spOffset(),
    allocs(),
    blocks(),
    fun(_fun),
    maxArgNum() {}

vector<KValueData *> Function::getAllKValueDatum() const {
    auto result = vector<KValueData *>();
    for (size_t i = 0; i < fun->params.len; ++i) {
        result.push_back((KValueData *) fun->params.buffer[i]);
    }
    for (size_t i = 0; i < fun->bbs.len; ++i) {
        auto bb = (KBasicBlock) (fun->bbs.buffer[i]);
        for (size_t j = 0; j < bb->insts.len; ++j) {
            auto inst = (bb->insts.buffer[j]);
            result.push_back((KValueData *) inst);
        }
    }
    return result;
}


vector<KBasicBlock> Function::gatAllKBasicBlocks() const {
    auto result = vector<KBasicBlock>();
    for (size_t i = 0; i < fun->bbs.len; ++i) {
        result.push_back((KBasicBlock) fun->bbs.buffer[i]);
    }
    return result;
}

const string &Function::getBlockName(const KBasicBlock &block) {
    return blocks[block];
}

void Function::captureArgNum(unsigned int argNum) {
    if ((!maxArgNum) || (argNum > maxArgNum.value())) {
        maxArgNum = argNum;
    }
}

void Function::allocSlot(const KValueData &val) {
    if (val.kind.tag == KOOPA_RVT_ALLOC) {
        allocs.emplace(&val, Slot{allocSize, false});
        allocSize += SizeOf(val.ty->data.pointer.base);
    } else {
        auto slot = Slot{allocSize, val.ty->tag == KOOPA_RTT_POINTER};
        allocs.emplace(&val, slot);
        allocSize += SizeOf(val.ty);
    }
}

optional<Slot> Function::getSlotOffset(const KValueData &val) {
    auto it = allocs.find(&val);
    if (it == allocs.end()) return nullopt;
    auto &original = it->second;
    if (!maxArgNum.has_value()) {
        return Slot{
            getSpOffset() - allocSize + original.offset,
            original.isPointer
        };
    } else {
        return Slot{
            getSpOffset() - allocSize + original.offset - 4,
            original.isPointer
        };
    }
}

void Function::captureBlockName(KBasicBlock const &block, const string &_name) {
    auto name = ".L" + dropFirst(_name) + "_" + to_string(assignNewLabel()) + "_" + dropFirst(fun->name);
    blocks.emplace(block, name);
}


unsigned int Function::getSpOffset() {
    if (spOffset) return spOffset.value();
    unsigned int args = maxArgNum && (maxArgNum.value() > 8) ? (maxArgNum.value() - 8) * 4 : 0;
    unsigned int ofs = allocSize + args + (maxArgNum.has_value() ? 4 : 0);
    spOffset = ((ofs + 15) / 16) * 16;
    return spOffset.value();
}

unsigned int Function::assignNewLabel() {
    return labelCount++;
}
