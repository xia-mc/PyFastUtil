
//
// Created by xia__mc on 2024/11/13.
//

#ifndef PYFASTUTIL_COMPAT_H
#define PYFASTUTIL_COMPAT_H

#pragma clang diagnostic push
#pragma ide diagnostic ignored "bugprone-reserved-identifier"

#if defined(__APPLE__) && defined(__cplusplus)
#include <string>
#include <sys/utsname.h>
#endif

#if defined(_MSC_VER)
#define __forceinline __forceinline
#elif defined(__GNUC__) || defined(__clang__)
#define __forceinline inline __attribute__((__always_inline__))
#else
#define __forceinline inline
#endif

#ifndef __cplusplus
#include "stdbool.h"
#endif

#if defined(_WIN64) || defined(WIN64)
#define WINDOWS64
#endif

#if (defined(_WIN32) || defined(WIN32)) && !defined(WINDOWS64)
#define WINDOWS32
#endif

#if defined(WINDOWS32) || defined(WINDOWS64) || defined(WINNT)
#define WINDOWS
#endif

#if PY_VERSION_HEX >= 0x030D0000  // 3.13
#define IS_PYTHON_313_OR_LATER
#endif

#if PY_VERSION_HEX >= 0x030C0000  // 3.12
#define IS_PYTHON_312_OR_LATER
#endif

#if PY_VERSION_HEX >= 0x03090000 // 3.9
#define IS_PYTHON_39_OR_LATER
#endif

#if !defined(Py_XNewRef) && defined(Py_XINCREF)  // 3.9
#define Py_XNewRef(obj) (Py_XINCREF(obj), obj)
#endif

#if defined(__GNUC__) || defined(__clang__)
#define LIKELY(x)   __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define LIKELY(x)   (x)
#define UNLIKELY(x) (x)
#endif

#ifdef __cplusplus
namespace compat {

    /**
     * Helper function to get the macOS version
     */
    [[maybe_unused]] static __forceinline bool isMacos1015OrNewer() {
#ifdef __APPLE__
        struct utsname sys_info;
        if (uname(&sys_info) != 0) {
            return false;  // If uname fails, assume older version
        }
        std::string version = sys_info.release;

        // macOS version is encoded in the release string as "19.x", "20.x", etc.
        // "19.x" corresponds to macOS 10.15, "20.x" corresponds to macOS 11, and so on.
        int major_version = std::stoi(version.substr(0, version.find('.')));
        return major_version >= 19;  // 19 corresponds to macOS 10.15
#else
        return false;
#endif
    }

}
#endif

#pragma clang diagnostic pop

#endif //PYFASTUTIL_COMPAT_H
