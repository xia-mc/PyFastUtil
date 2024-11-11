//
// Created by xia__mc on 2024/11/11.
//

#ifndef PYFASTUTIL_SIMDSORT_H
#define PYFASTUTIL_SIMDSORT_H

#include <vector>
#include <algorithm>
#include <immintrin.h>
#include <iostream>

namespace simd {

    void simd_sort(std::vector<int>::iterator begin, std::vector<int>::iterator end, const bool &reverse = false);

}


#endif //PYFASTUTIL_SIMDSORT_H
