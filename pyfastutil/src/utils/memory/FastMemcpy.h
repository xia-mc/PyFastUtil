//
// Created by xia__mc on 2024/12/10.
//

#ifndef PYFASTUTIL_FASTMEMCPY_H
#define PYFASTUTIL_FASTMEMCPY_H

#include <cstring>
#include "Compat.h"
#include "utils/simd/SIMDUtils.h"

__forceinline void fast_memcpy(void *__restrict dst, const void *__restrict src, size_t size) {
    switch (size) {
        case 0:
            return;
        case 1:
            memcpy(dst, src, 1);
            break;
        case 2:
            memcpy(dst, src, 2);
            break;
        case 3:
            memcpy(dst, src, 3);
            break;
        case 4:
            memcpy(dst, src, 4);
            break;
        case 5:
            memcpy(dst, src, 5);
            break;
        case 6:
            memcpy(dst, src, 6);
            break;
        case 7:
            memcpy(dst, src, 7);
            break;
        case 8:
            memcpy(dst, src, 8);
            break;
        case 9:
            memcpy(dst, src, 9);
            break;
        case 10:
            memcpy(dst, src, 10);
            break;
        case 11:
            memcpy(dst, src, 11);
            break;
        case 12:
            memcpy(dst, src, 12);
            break;
        case 13:
            memcpy(dst, src, 13);
            break;
        case 14:
            memcpy(dst, src, 14);
            break;
        case 15:
            memcpy(dst, src, 15);
            break;
        case 16:
            memcpy(dst, src, 16);
            break;
        case 17:
            memcpy(dst, src, 17);
            break;
        case 18:
            memcpy(dst, src, 18);
            break;
        case 19:
            memcpy(dst, src, 19);
            break;
        case 20:
            memcpy(dst, src, 20);
            break;
        case 21:
            memcpy(dst, src, 21);
            break;
        case 22:
            memcpy(dst, src, 22);
            break;
        case 23:
            memcpy(dst, src, 23);
            break;
        case 24:
            memcpy(dst, src, 24);
            break;
        case 25:
            memcpy(dst, src, 25);
            break;
        case 26:
            memcpy(dst, src, 26);
            break;
        case 27:
            memcpy(dst, src, 27);
            break;
        case 28:
            memcpy(dst, src, 28);
            break;
        case 29:
            memcpy(dst, src, 29);
            break;
        case 30:
            memcpy(dst, src, 30);
            break;
        case 31:
            memcpy(dst, src, 31);
            break;
        case 32:
            memcpy(dst, src, 32);
            break;
        default:
            if ((uintptr_t) src % 64 == 0 && (uintptr_t) dst % 64 == 0) {
                simd::simdMemCpyAligned((char *) src, (char *) dst, size);
            } else {
                simd::simdMemCpy((char *) src, (char *) dst, size);
            }
            break;
    }
}

#endif //PYFASTUTIL_FASTMEMCPY_H
