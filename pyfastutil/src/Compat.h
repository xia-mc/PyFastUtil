
//
// Created by xia__mc on 2024/11/13.
//

#ifndef PYFASTUTIL_COMPAT_H
#define PYFASTUTIL_COMPAT_H

#pragma clang diagnostic push
#pragma ide diagnostic ignored "bugprone-reserved-identifier"

#ifdef __APPLE__
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


namespace compat {

    /**
     * Helper function to get the macOS version
     */
    static __forceinline bool isMacos1015OrNewer() {
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

#pragma clang diagnostic pop

#endif //PYFASTUTIL_COMPAT_H
