
//
// Created by xia__mc on 2024/11/13.
//

#ifndef PYFASTUTIL_COMPAT_H
#define PYFASTUTIL_COMPAT_H

#pragma clang diagnostic push
#pragma ide diagnostic ignored "bugprone-reserved-identifier"

#if defined(_MSC_VER)
#define __forceinline __forceinline
#elif defined(__GNUC__) || defined(__clang__)
#define __forceinline inline __attribute__((__always_inline__))
#else
#define __forceinline inline
#endif

#endif //PYFASTUTIL_COMPAT_H

#pragma clang diagnostic pop
