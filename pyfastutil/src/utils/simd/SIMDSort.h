//
// Created by xia__mc on 2024/11/11.
//

#ifndef PYFASTUTIL_SIMDSORT_H
#define PYFASTUTIL_SIMDSORT_H

#include <vector>
#include <algorithm>
#include <immintrin.h>
#include <iostream>
#include "utils/memory/AlignedAllocator.h"

namespace simd {

    void simdsort(std::vector<int, AlignedAllocator<int, 64>>& vector, const bool &reverse = false);

}


#endif //PYFASTUTIL_SIMDSORT_H
