//
// Created by xia__mc on 2024/12/10.
//

#include "ExceptionHandler.h"
#include "utils/PythonPCH.h"
#include "Compat.h"

#ifdef WINDOWS

#include "windows.h"

#ifdef WIN64

static const unsigned char ret_instruction[] = {0xC3};  // amd64 ret; i386 ret (cdecl)

static LONG CALLBACK VectoredHandler(PEXCEPTION_POINTERS exceptionInfo) {
    if (exceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION) {
        OutputDebugStringA("[PyFastUtil Exception Handler] Access violation caught in VEH!\n");

        const PyGILState_STATE gilState = PyGILState_Ensure();
        if (!PyErr_Occurred())
            PyErr_SetString(PyExc_RuntimeError, "Access violation caught in VEH!");
        PyGILState_Release(gilState);

        // return NULL;
        exceptionInfo->ContextRecord->Rax = (DWORD64) NULL;
        exceptionInfo->ContextRecord->Rip = (DWORD64) ret_instruction;

        return EXCEPTION_CONTINUE_EXECUTION;
    }

    return EXCEPTION_CONTINUE_SEARCH;
}

#else

static LONG CALLBACK VectoredHandler(PEXCEPTION_POINTERS exceptionInfo) {
    if (exceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION) {
        OutputDebugStringA("[PyFastUtil Exception Handler] Access violation caught in VEH! "
                           "Global exception handler is unavailable on i386.\n");

        const PyGILState_STATE gilState = PyGILState_Ensure();
        if (!PyErr_Occurred())
            PyErr_SetString(PyExc_RuntimeError, "Access violation caught in VEH! "
                                                "Global exception handler is unavailable on i386.");
        PyGILState_Release(gilState);
    }

    return EXCEPTION_CONTINUE_SEARCH;
}

#endif

void initExceptionHandler() {
#ifdef WIN64
    DWORD oldProtect;
    if (!VirtualProtect((LPVOID) ret_instruction, (SIZE_T) sizeof(ret_instruction),
                        PAGE_EXECUTE_READWRITE, &oldProtect)) {
        fprintf(stderr, "Failed to make ret_instruction executable. Error code: %lu\n", GetLastError());
        fputs("Global exception handler is unavailable.", stderr);
        return;
    }
#endif
    AddVectoredExceptionHandler(1, VectoredHandler);
}

#else

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <execinfo.h>
#include <ucontext.h>
#include <unistd.h>

// store orignal handler
static struct sigaction old_sigsegv_handler;

void segfault_handler(int sig, siginfo_t* info, void* context) {
    fputs("[PyFastUtil Exception Handler] Access violation caught in VEH! "
          "Global exception handler is unavailable on GNU/Linux.\n", stderr);

    const PyGILState_STATE gil_state = PyGILState_Ensure();
    if (!PyErr_Occurred())
        PyErr_SetString(PyExc_RuntimeError, "Access violation caught in VEH! "
                                            "Global exception handler is unavailable on GNU/Linux.");
    PyGILState_Release(gil_state);

    sigaction(SIGSEGV, &old_sigsegv_handler, NULL);
    raise(sig);
}

void initExceptionHandler() {
    struct sigaction sa;

    sa.sa_sigaction = segfault_handler;
    sa.sa_flags = SA_SIGINFO | SA_RESETHAND;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGSEGV, &sa, &old_sigsegv_handler) != 0) {
        perror("Failed to set signal handler for SIGSEGV");
        perror("Global exception handler is unavailable.");
        return;
    }

    fprintf(stderr, "Custom SIGSEGV handler installed.\n");
}

#endif

