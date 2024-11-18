import glob
import os
import platform
import shutil

from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext

IS_WINDOWS = os.name == "nt"
IS_MACOS = platform.system() == "Darwin"

if IS_WINDOWS:
    # Windows uses MSVC
    EXTRA_COMPILE_ARG = [
        "/O2", "/Ob2", "/GL", "/W4", "/std:c++latest", "/WX", "/MP",
        "/wd4068",  # ignore unknown pragma error
        "/EHsc"
    ]
    EXTRA_LINK_ARG = ["/LTCG"]
else:
    # Unix-like uses GCC/Clang
    EXTRA_COMPILE_ARG = [
        "-O3", "-flto", "-fPIC",
        "-std=c++2b", "-Wall", "-fvisibility=hidden",
        "-Wno-error=unknown-pragmas",
        "-mavx", "-mavx2", "-mavx512f", "-mavx512bw", "-mavx512dq", "-mavx512vl",
        "-fno-tree-vectorize"
    ]
    EXTRA_LINK_ARG = ["-flto"]

    if IS_MACOS:
        EXTRA_COMPILE_ARG.append("-faligned-allocation")


class CustomBuildExt(build_ext):
    def build_extensions(self):
        if IS_MACOS:
            C_EXTRA_COMPILE_ARG = ["-std=c23" if arg == "-std=c++2b" else "-std=c++2b" for arg in EXTRA_COMPILE_ARG]
        else:
            C_EXTRA_COMPILE_ARG = EXTRA_COMPILE_ARG

        for ext in self.extensions:
            for i, source in enumerate(ext.sources):
                if source.endswith(".c"):
                    ext.extra_compile_args = C_EXTRA_COMPILE_ARG
                elif source.endswith(".cpp"):
                    ext.extra_compile_args = EXTRA_COMPILE_ARG
        build_ext.build_extensions(self)


if __name__ == "__main__":
    if os.path.exists("./build"):
        shutil.rmtree("./build")

    # init sources
    sources = []
    for root, dirs, files in os.walk("./pyfastutil/src"):
        for file in files:
            if file.endswith(".cpp") or file.endswith(".c"):
                sources.append(os.path.join(root, file))

    modules = [
        Extension(
            name="__pyfastutil",
            sources=sources,
            language="c++",
            extra_compile_args=EXTRA_COMPILE_ARG,
            extra_link_args=EXTRA_LINK_ARG,
            include_dirs=["./pyfastutil/src"]
        )
    ]

    # build
    setup(
        name="__pyfastutil",
        version="0.0.1",
        description="C++ implementation of PyFastUtil.",
        ext_modules=modules,
        cmdclass={'build_ext': CustomBuildExt}
    )

    # output
    pattern: str
    pattern = "./build/lib.*/__pyfastutil.*"
    files = glob.glob(pattern)

    if files:
        sourceFile = files[0]
        targetFile = os.path.join("./pyfastutil", "__pyfastutil" + (".pyd" if IS_WINDOWS else ".so"))

        shutil.move(sourceFile, targetFile)
        print(f"File moved and renamed from {sourceFile} to {targetFile}")
    else:
        print("No files found matching the pattern")
