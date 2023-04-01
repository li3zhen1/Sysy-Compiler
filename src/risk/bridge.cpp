//
// Created by 李臻 on 2022/5/24.
//
#include "bridge.h"

bool risk::isIMM12(int x) {
    return x >= -2048 && x <= 2047;
}

string risk::dropFirst(const string &str) {
    return {str.begin() + 1, str.end()};
}

unsigned int risk::SizeOf(const KType &t) {
    switch (t->tag) {
        case KOOPA_RTT_INT32:
        case KOOPA_RTT_POINTER:
        case KOOPA_RTT_FUNCTION:
            return 4;
        case KOOPA_RTT_ARRAY:
            return SizeOf(t->data.array.base) * t->data.array.len;
        case KOOPA_RTT_UNIT:
            return 0;
    }
}

bool risk::isGlobal(koopa_raw_value_tag_t tag) {
    return tag < 6 || tag == 7;
}

bool risk::isLocal(koopa_raw_value_tag_t tag) {
    return tag == 6 || tag >= 8;
}