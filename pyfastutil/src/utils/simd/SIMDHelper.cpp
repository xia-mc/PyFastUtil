//
// Created by xia__mc on 2024/11/11.
//

#include "SIMDHelper.h"

// Helper function to check OS support for AVX (via XGETBV)
bool osSupportsAVX();

// Function to check if AVX2 is supported
bool isAVX2Supported();

// Function to check if AVX-512 is supported
bool isAVX512Supported();

// Function to check if SSE4.1 is supported
bool isSSE41Supported();

// Function to check if SSSE3 is supported
bool isSSSE3Supported();

// Function to check if Neon is supported
bool isArmNeonSupported();

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)

#if defined(_MSC_VER)

#include <intrin.h>  // For __cpuid() and _xgetbv()

bool osSupportsAVX() {
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

bool isAVX2Supported() {
    int cpuInfo[4];

    // First, check if the CPU supports AVX and XSAVE (CPUID leaf 1, ECX bit 28 and ECX bit 27)
    __cpuid(cpuInfo, 1); // CPUID leaf 1
    bool avx_supported = (cpuInfo[2] & (1 << 28)) != 0;  // Check AVX bit (bit 28 in ECX)
    bool xsave_supported = (cpuInfo[2] & (1 << 27)) != 0; // Check XSAVE bit (bit 27 in ECX)
    if (!avx_supported || !xsave_supported) {
        return false; // AVX or XSAVE not supported
    }

    // Check OS support for AVX using XGETBV
    if (!osSupportsAVX()) {
        return false; // OS does not support AVX
    }

    // Now check for AVX2 support (CPUID leaf 7, sub-leaf 0, EBX bit 5)
    __cpuidex(cpuInfo, 7, 0); // CPUID leaf 7, sub-leaf 0
    return (cpuInfo[1] & (1 << 5)) != 0; // Check if AVX2 is supported (bit 5 in EBX)
}

bool isAVX512Supported() {
    int cpuInfo[4];

    // First, check if the CPU supports AVX and XSAVE (CPUID leaf 1, ECX bit 28 and ECX bit 27)
    __cpuid(cpuInfo, 1); // CPUID leaf 1
    bool avx_supported = (cpuInfo[2] & (1 << 28)) != 0;  // Check AVX bit (bit 28 in ECX)
    bool xsave_supported = (cpuInfo[2] & (1 << 27)) != 0; // Check XSAVE bit (bit 27 in ECX)
    if (!avx_supported || !xsave_supported) {
        return false; // AVX or XSAVE not supported
    }

    // Check OS support for AVX using XGETBV
    if (!osSupportsAVX()) {
        return false; // OS does not support AVX
    }

    // Now check for AVX-512 support (CPUID leaf 7, sub-leaf 0, EBX bit 16)
    __cpuidex(cpuInfo, 7, 0); // CPUID leaf 7, sub-leaf 0
    return (cpuInfo[1] & (1 << 16)) != 0; // Check if AVX-512 is supported (bit 16 in EBX)
}

bool isSSE41Supported() {
    int cpuInfo[4] = {0};

    // get cpu feature info
    __cpuid(cpuInfo, 1);

    // ECX 19
    bool sse41Supported = (cpuInfo[2] & (1 << 19)) != 0;

    return sse41Supported;
}

bool isSSSE3Supported() {
    int cpuInfo[4] = {0};

    // Get CPU feature info using __cpuid
    __cpuid(cpuInfo, 1);

    // Check ECX bit 9 (SSSE3 support)
    bool ssse3Supported = (cpuInfo[2] & (1 << 9)) != 0;

    return ssse3Supported;
}

bool isArmNeonSupported() {
    return false;
}

#elif defined(__GNUC__) || defined(__clang__)

#include <cpuid.h>  // For __get_cpuid() and __get_cpuid_count()

bool osSupportsAVX() {
    // Ensure CPU supports XGETBV (XSAVE/XRSTOR)
    unsigned int cpuid_eax, cpuid_ebx, cpuid_ecx, cpuid_edx;
    __get_cpuid(1, &cpuid_eax, &cpuid_ebx, &cpuid_ecx, &cpuid_edx);

    // Check if XSAVE/XRSTOR is supported (bit 27 of ECX)
    if (!(cpuid_ecx & (1 << 27))) {
        return false; // XSAVE not supported, so XGETBV cannot be used
    }
#if defined(__APPLE__)  // IDK why
    return false;
#elif defined(__GNUC__) && (__GNUC__ >= 8)
    // GCC 8+ has __builtin_ia32_xgetbv()
    unsigned long long xcrFeatureMask = __builtin_ia32_xgetbv(0);
    // Check if XMM (bit 1) and YMM (bit 2) state are enabled by OS
    return (xcrFeatureMask & 0x6) == 0x6; // Both XMM and YMM must be enabled
#else
    __asm__ __volatile__(
            "xgetbv"
            : "=a"(eax), "=d"(edx)
            : "c"(0)
        );
    unsigned long long xcrFeatureMask = ((unsigned long long)edx << 32) | eax;
    // Check if XMM (bit 1) and YMM (bit 2) state are enabled by OS
    return (xcrFeatureMask & 0x6) == 0x6; // Both XMM and YMM must be enabled
#endif
}

bool isAVX2Supported() {
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
    if (!osSupportsAVX()) {
        return false; // OS does not support AVX
    }

    // Now check for AVX2 support (CPUID leaf 7, sub-leaf 0, EBX bit 5)
    if (!__get_cpuid_count(7, 0, &eax, &ebx, &ecx, &edx)) {
        return false;
    }

    return (ebx & (1 << 5)) != 0; // Check if AVX2 is supported
}

bool isAVX512Supported() {
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
    if (!osSupportsAVX()) {
        return false; // OS does not support AVX
    }

    // Now check for AVX-512 support (CPUID leaf 7, sub-leaf 0, EBX bit 16)
    if (!__get_cpuid_count(7, 0, &eax, &ebx, &ecx, &edx)) {
        return false;
    }

    return (ebx & (1 << 16)) != 0; // Check if AVX-512 is supported
}

bool isSSE41Supported() {
    unsigned int eax, ebx, ecx, edx;

    if (__get_cpuid(1, &eax, &ebx, &ecx, &edx)) {
        bool sse41Supported = (ecx & (1 << 19)) != 0;
        return sse41Supported;
    }

    return false;  // failed to get cpu info
}

bool isSSSE3Supported() {
    unsigned int eax, ebx, ecx, edx;

    if (__get_cpuid(1, &eax, &ebx, &ecx, &edx)) {
        bool ssse3Supported = (ecx & (1 << 9)) != 0;
        return ssse3Supported;
    }

    return false;  // Failed to get CPU info
}

bool isArmNeonSupported() {
    return false;
}

#else
// For unknown compiler, SIMD support is unknown, default is unsupported.
bool osSupportsAVX() {
    return false;
}

bool isAVX2Supported() {
    return false;
}

bool isAVX512Supported() {
    return false;
}

bool isSSE41Supported() {
    return false;
}

bool isSSSE3Supported() {
    return false;
}

bool isArmNeonSupported() {
    return false;
}

#endif

#else

#include <fstream>
#include <string>

// For non-x86 architectures, AVX/SSE is not supported
bool osSupportsAVX() {
    return false;
}

bool isAVX2Supported() {
    return false;
}

bool isAVX512Supported() {
    return false;
}

bool isSSE41Supported() {
    return false;
}

bool isSSSE3Supported() {
    return false;
}

bool isArmNeonSupported() {
    std::ifstream cpuinfo("/proc/cpuinfo");
    if (!cpuinfo.is_open()) {
        return false;  // Failed to open /proc/cpuinfo
    }

    std::string line;
    while (std::getline(cpuinfo, line)) {
        // Look for the "Features" line and check if it contains "neon"
        if (line.find("Features") != std::string::npos && line.find("neon") != std::string::npos) {
            return true;  // NEON is supported
        }
    }

    return false;  // NEON is not found in the features
}

#endif


namespace simd {
    const bool IS_AVX2_SUPPORTED = isAVX2Supported();
    const bool IS_AVX512_SUPPORTED = isAVX512Supported();
    const bool IS_SSE41_SUPPORTED = isSSE41Supported();
    const bool IS_SSSE3_SUPPORTED = isSSSE3Supported();
    const bool IS_ARM_NEON_SUPPORTED = isArmNeonSupported();
}
