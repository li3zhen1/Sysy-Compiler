//
// Created by 李臻 on 2022/5/23.
//

#pragma once


#include <optional>
#include <unordered_map>
#include <string>
#include <vector>


#include "bridge.h"

using namespace std;

namespace risk {

    class Function {

        unsigned int labelCount = 0;
        unsigned int allocSize;
        optional<unsigned int> spOffset;
        unordered_map<const KValueData *, Slot> allocs;
        unordered_map<KBasicBlock, string> blocks;

    public:
        KFunction fun;

        unsigned int assignNewLabel();

        explicit Function(const KFunction &fun);

        void captureArgNum(unsigned int argNum);

        void allocSlot(const KValueData &val);

        optional<Slot> getSlotOffset(const KValueData &val);

        void captureBlockName(const KBasicBlock &block, const string &name);

        const string &getBlockName(const KBasicBlock &block);

        unsigned int getSpOffset();

        [[nodiscard]] vector<KValueData *> getAllKValueDatum() const;

        [[nodiscard]] vector<KBasicBlock> gatAllKBasicBlocks() const;

        optional<unsigned int> maxArgNum;
    };
}