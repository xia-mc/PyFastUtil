//
// Created by xia__mc on 2024/11/11.
//

#include "SIMD.h"

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)

#if defined(_MSC_VER)

#include <intrin.h>  // For __cpuid() and _xgetbv()

// Helper function to check OS support for AVX (via XGETBV)
bool os_supports_avx() {
    int cpuInfo[4];
    __cpuid(cpuInfo, 1);

    // Check if XSAVE/XRSTOR is supported (bit 27 of ECX)
    if (!(cpuInfo[2] & (1 << 27))) {
        return false; // XSAVE not supported
    }

    // Use _xgetbv to check if OS supports AVX (XMM and YMM)
    unsigned long long xcrFeatureMask = _xgetbv(0);

    // Check if XMM (bit 1) and YMM (bit 2) state are enabled by OS
    return (xcrFeatureMask & 0x6) == 0x6;
}

// Function to check if AVX2 is supported
bool is_avx2_supported() {
    int cpuInfo[4];

    // First, check if the CPU supports AVX and XSAVE (CPUID leaf 1, ECX bit 28 and ECX bit 27)
    __cpuid(cpuInfo, 1); // CPUID leaf 1
    bool avx_supported = (cpuInfo[2] & (1 << 28)) != 0;  // Check AVX bit (bit 28 in ECX)
    bool xsave_supported = (cpuInfo[2] & (1 << 27)) != 0; // Check XSAVE bit (bit 27 in ECX)
    if (!avx_supported || !xsave_supported) {
        return false; // AVX or XSAVE not supported
    }

    // Check OS support for AVX using XGETBV
    if (!os_supports_avx()) {
        return false; // OS does not support AVX
    }

    // Now check for AVX2 support (CPUID leaf 7, sub-leaf 0, EBX bit 5)
    __cpuidex(cpuInfo, 7, 0); // CPUID leaf 7, sub-leaf 0
    return (cpuInfo[1] & (1 << 5)) != 0; // Check if AVX2 is supported (bit 5 in EBX)
}

// Function to check if AVX-512 is supported
bool is_avx512_supported() {
    int cpuInfo[4];

    // First, check if the CPU supports AVX and XSAVE (CPUID leaf 1, ECX bit 28 and ECX bit 27)
    __cpuid(cpuInfo, 1); // CPUID leaf 1
    bool avx_supported = (cpuInfo[2] & (1 << 28)) != 0;  // Check AVX bit (bit 28 in ECX)
    bool xsave_supported = (cpuInfo[2] & (1 << 27)) != 0; // Check XSAVE bit (bit 27 in ECX)
    if (!avx_supported || !xsave_supported) {
        return false; // AVX or XSAVE not supported
    }

    // Check OS support for AVX using XGETBV
    if (!os_supports_avx()) {
        return false; // OS does not support AVX
    }

    // Now check for AVX-512 support (CPUID leaf 7, sub-leaf 0, EBX bit 16)
    __cpuidex(cpuInfo, 7, 0); // CPUID leaf 7, sub-leaf 0
    return (cpuInfo[1] & (1 << 16)) != 0; // Check if AVX-512 is supported (bit 16 in EBX)
}

#else

#include <cpuid.h>  // For __get_cpuid() and __get_cpuid_count()

// Helper function to check OS support for AVX (via XGETBV)
bool os_supports_avx() {
    // Ensure CPU supports XGETBV (XSAVE/XRSTOR)
    unsigned int cpuid_eax, cpuid_ebx, cpuid_ecx, cpuid_edx;
    __get_cpuid(1, &cpuid_eax, &cpuid_ebx, &cpuid_ecx, &cpuid_edx);

    // Check if XSAVE/XRSTOR is supported (bit 27 of ECX)
    if (!(cpuid_ecx & (1 << 27))) {
        return false; // XSAVE not supported, so XGETBV cannot be used
    }
#if defined(__GNUC__) && (__GNUC__ >= 8)
    // GCC 8+ has __builtin_ia32_xgetbv()
    unsigned long long xcrFeatureMask = __builtin_ia32_xgetbv(0);
#else
    __asm__ __volatile__(
            "xgetbv"
            : "=a"(eax), "=d"(edx)
            : "c"(0)
        );
        unsigned long long xcrFeatureMask = ((unsigned long long)edx << 32) | eax;
#endif
    // Check if XMM (bit 1) and YMM (bit 2) state are enabled by OS
    return (xcrFeatureMask & 0x6) == 0x6; // Both XMM and YMM must be enabled
}

// Function to check if AVX2 is supported
bool is_avx2_supported() {
    unsigned int eax, ebx, ecx, edx;

    // First, check if the CPU supports AVX and XSAVE (CPUID leaf 1, ECX bit 28 and ECX bit 27)
    if (!__get_cpuid(1, &eax, &ebx, &ecx, &edx)) {
        return false; // CPUID instruction failed
    }

    bool avx_supported = (ecx & (1 << 28)) != 0;  // Check AVX bit
    bool xsave_supported = (ecx & (1 << 27)) != 0; // Check XSAVE bit
    if (!avx_supported || !xsave_supported) {
        return false; // AVX or XSAVE not supported
    }

    // Check OS support for AVX using XGETBV
    if (!os_supports_avx()) {
        return false; // OS does not support AVX
    }

    // Now check for AVX2 support (CPUID leaf 7, sub-leaf 0, EBX bit 5)
    if (!__get_cpuid_count(7, 0, &eax, &ebx, &ecx, &edx)) {
        return false;
    }

    return (ebx & (1 << 5)) != 0; // Check if AVX2 is supported
}

// Function to check if AVX-512 is supported
bool is_avx512_supported() {
    unsigned int eax, ebx, ecx, edx;

    // First, check if the CPU supports AVX and XSAVE (CPUID leaf 1, ECX bit 28 and ECX bit 27)
    if (!__get_cpuid(1, &eax, &ebx, &ecx, &edx)) {
        return false; // CPUID instruction failed
    }

    bool avx_supported = (ecx & (1 << 28)) != 0;  // Check AVX bit
    bool xsave_supported = (ecx & (1 << 27)) != 0; // Check XSAVE bit
    if (!avx_supported || !xsave_supported) {
        return false; // AVX or XSAVE not supported
    }

    // Check OS support for AVX using XGETBV
    if (!os_supports_avx()) {
        return false; // OS does not support AVX
    }

    // Now check for AVX-512 support (CPUID leaf 7, sub-leaf 0, EBX bit 16)
    if (!__get_cpuid_count(7, 0, &eax, &ebx, &ecx, &edx)) {
        return false;
    }

    return (ebx & (1 << 16)) != 0; // Check if AVX-512 is supported
}


#endif

#else
// For non-x86 architectures, AVX is not supported
constexpr bool os_supports_avx() {
    return false;
}

bool is_avx2_supported() {
    return false;
}

bool is_avx512_supported() {
    return false;
}
#endif


namespace simd {
    const bool IS_AVX2_SUPPORTED = is_avx2_supported();
    const bool IS_AVX512_SUPPORTED = is_avx512_supported();
}