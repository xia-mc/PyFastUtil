import glob
import os
import shutil

from setuptools import setup, Extension

IS_WINDOWS = os.name == "nt"

if IS_WINDOWS:
    # Windows uses MSVC
    EXTRA_COMPILE_ARG = [
        "/O2", "/Ob2", "/GL", "/W4", "/std:c++latest", "/WX",
        "/wd4068",  # ignore unknown pragma error
        "/EHsc"
    ]
    EXTRA_LINK_ARG = []
else:
    # Unix-like uses GCC/Clang
    EXTRA_COMPILE_ARG = [
        "-O3", "-funroll-loops", "-flto", "-fPIC",
        "-std=c++2b", "-Wall", "-fvisibility=hidden",
        "-Wno-error=unknown-pragmas",
        "-mavx", "-mavx2", "-mavx512f", "-mavx512bw", "-mavx512dq", "-mavx512vl",
        "-fno-tree-vectorize"
        # "-Wno-error=unguarded-availability-new"  # already handle in Compat.h, and this option doesn't exist on gcc
    ]
    EXTRA_LINK_ARG = []

if __name__ == "__main__":
    if os.path.exists("./build"):
        shutil.rmtree("./build")

    # init sources
    sources = []
    for root, dirs, files in os.walk("./pyfastutil/src"):
        for file in files:
            if file.endswith(".cpp"):
                sources.append(os.path.join(root, file))

    module1 = Extension(
        name="__pyfastutil",
        sources=sources,
        language="c++",
        extra_compile_args=EXTRA_COMPILE_ARG,
        extra_link_args=EXTRA_LINK_ARG,
        include_dirs=["./pyfastutil/src"]
    )

    # build
    setup(
        name="__pyfastutil",
        version="0.0.1",
        description="C++ implementation of PyFastUtil.",
        ext_modules=[module1]
    )

    # output
    pattern: str
    pattern = "./build/lib.*-cpython-*/__pyfastutil.*"
    files = glob.glob(pattern)

    if files:
        sourceFile = files[0]
        targetFile = os.path.join("./pyfastutil", "__pyfastutil" + (".pyd" if IS_WINDOWS else ".so"))

        shutil.move(sourceFile, targetFile)
        print(f"File moved and renamed from {sourceFile} to {targetFile}")
    else:
        print("No files found matching the pattern")
