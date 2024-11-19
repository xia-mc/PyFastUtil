//
// Created by xia__mc on 2024/11/19.
//

#ifndef PYFASTUTIL_UTILS_H
#define PYFASTUTIL_UTILS_H

#include "list"
#include "Compat.h"

template<typename T>
static __forceinline typename std::list<T>::iterator at(std::list<T> &list, const size_t index) {
    const auto size = list.size();

    if (index >= size) {
        return list.end();
    }

    if (index > size / 2) {
        auto result = std::prev(list.end());
        const auto toMove = size - index - 1;
        for (size_t i = 0; i < toMove; ++i) {
            --result;
        }
        return result;
    } else {
        auto result = list.begin();
        for (size_t i = 0; i < index; ++i) {
            ++result;
        }
        return result;
    }
}

#endif //PYFASTUTIL_UTILS_H
