//
// Created by xia__mc on 2024/11/11.
//

#ifndef PYFASTUTIL_BITONICSORT_H
#define PYFASTUTIL_BITONICSORT_H

#include <vector>
#include "utils/memory/AlignedAllocator.h"
#include "Compat.h"

namespace simd {

    void initBitonicSort();

    void simdsort(std::vector<int, AlignedAllocator<int, 64>> &vector, bool reverse);

    void simdsort(std::vector<long long, AlignedAllocator<long long, 64>> &vector, bool reverse);
}


#endif //PYFASTUTIL_BITONICSORT_H
