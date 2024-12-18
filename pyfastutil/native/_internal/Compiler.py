import glob
import os
import platform
import shutil
from pathlib import Path
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext

H_TEMPLATE = """
//
// Created by xia__mc on 2024/12/16.
//

#ifndef PYFASTUTIL_NATIVECODEBASE_H
#define PYFASTUTIL_NATIVECODEBASE_H

#define MODULE_NAME %MODULE_NAME%

#include "utils/PythonPCH.h"

PyMODINIT_FUNC PyInit_%MODULE_NAME%();

#endif //PYFASTUTIL_NATIVECODEBASE_H
"""

CPP_TEMPLATE = """
//
// Created by xia__mc on 2024/12/16.
//

#include "%MODULE_NAME%.h"
#include "utils/PythonUtils.h"

static PyObject *%MODULE_NAME%_function([[maybe_unused]] PyObject *self, 
                                        [[maybe_unused]] PyObject *const *args, [[maybe_unused]] Py_ssize_t nargs) noexcept {
    [[maybe_unused]] PyObject *res;

    %CODE%

    Py_UNREACHABLE();
}

static PyMethodDef %MODULE_NAME%_methods[] = {
    {"%FUNC_NAME%", (PyCFunction) %MODULE_NAME%_function, METH_FASTCALL, "<native method>"},
    {nullptr, nullptr, 0, nullptr}
};

static struct PyModuleDef %MODULE_NAME%_module = {
        PyModuleDef_HEAD_INIT,
        "<native_module>",
        nullptr,
        -1,
        %MODULE_NAME%_methods, nullptr, nullptr, nullptr, nullptr
};

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
PyMODINIT_FUNC PyInit_%MODULE_NAME%() {
    PyObject *object = PyModule_Create(&%MODULE_NAME%_module);
    if (object == nullptr)
        return nullptr;

    return object;
}
#pragma clang diagnostic pop
"""

IS_WINDOWS = os.name == "nt"
IS_MACOS = platform.system() == "Darwin"

if IS_WINDOWS:
    EXTRA_COMPILE_ARG = [
        "/O2", "/Ob2", "/GL", "/W4", "/std:c++20", "/WX",
        "/wd4068",  # ignore unknown pragma error
        "/EHsc"
    ]
    EXTRA_LINK_ARG = ["/LTCG"]
elif IS_MACOS:
    EXTRA_COMPILE_ARG = [
        "-O3", "-flto", "-fPIC",
        "-Wall", "-fvisibility=hidden",
        "-Wno-error=unknown-pragmas",
        "-mavx", "-mavx2", "-mavx512f", "-mavx512bw", "-mavx512dq", "-mavx512vl",
        "-fno-tree-vectorize", "-faligned-allocation"
    ]
    EXTRA_LINK_ARG = ["-flto"]

    os.environ["CFLAGS"] = "-std=c2x"
    os.environ["CXXFLAGS"] = "-std=c++20"
else:
    EXTRA_COMPILE_ARG = [
        "-O3", "-flto", "-fPIC",
        "-std=c++20", "-Wall", "-fvisibility=hidden",
        "-Wno-error=unknown-pragmas",
        "-mavx", "-mavx2", "-mavx512f", "-mavx512bw", "-mavx512dq", "-mavx512vl",
        "-fno-tree-vectorize"
    ]
    EXTRA_LINK_ARG = ["-flto", "-fpermissive"]


def compileCode(cacheFolder: Path, funcName: str, moduleName: str, code: list[str]) -> None:
    # generate
    hStr = (H_TEMPLATE
            .replace("%MODULE_NAME%", moduleName))
    cppStr = (CPP_TEMPLATE
              .replace("%FUNC_NAME%", funcName)
              .replace("%MODULE_NAME%", moduleName)
              .replace("%CODE%", "\n    ".join(code)))

    hPath = Path(cacheFolder, f"{moduleName}.h")
    with open(hPath, "w") as f:
        f.write(hStr)

    cppPath = Path(cacheFolder, f"{moduleName}.cpp")
    with open(cppPath, "w") as f:
        f.write(cppStr)

    # compile
    sources = [str(cppPath.absolute())]

    modules = [
        Extension(
            name=moduleName,
            sources=sources,
            language="c++",
            extra_compile_args=EXTRA_COMPILE_ARG,
            extra_link_args=EXTRA_LINK_ARG,
            include_dirs=[
                str(Path(Path(__file__).parent.parent.parent, "src").absolute()),  # just prototype impl, so ignore this
                str(cacheFolder.absolute())
            ]
        )
    ]

    class CustomBuildExtCommand(build_ext):
        def run(self):
            build_ext.run(self)
            pattern = os.path.join(str(cacheFolder.absolute()), f"{moduleName}.*")
            outFiles = glob.glob(pattern)

            if outFiles:
                source_file = outFiles[0]
                target_filename = moduleName + ".pyd"
                target_file = os.path.join(str(cacheFolder.absolute()), target_filename)
                shutil.copy(source_file, target_file)
                os.remove(source_file)
                print(f"File copied and renamed from {source_file} to {target_file}")
            else:
                print("No files found matching the pattern")

    setup(
        name=moduleName,
        description="<native module>",
        ext_modules=modules,
        cmdclass={
            "build_ext": CustomBuildExtCommand,
        },
        package_data={
            moduleName: ["src/**/*.h"],
        },
        script_args=["build", f"--build-lib={cacheFolder}"]
    )

    # os.remove(cppPath)
    os.remove(hPath)

    return
