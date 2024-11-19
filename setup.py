import glob
import os
import platform
import shutil
from setuptools import setup, Extension, Command
from setuptools.command.build_ext import build_ext

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
    EXTRA_LINK_ARG = ["-flto"]


class CleanCommand(Command):
    user_options = []

    def initialize_options(self):
        pass

    def finalize_options(self):
        pass

    def run(self):
        shutil.rmtree("./build", ignore_errors=True)
        shutil.rmtree("./dist", ignore_errors=True)
        shutil.rmtree("./__pycache__", ignore_errors=True)
        shutil.rmtree("./pyfastutil.egg-info", ignore_errors=True)
        print("Cleaned up build directories.")


class CustomBuildExtCommand(build_ext):
    def run(self):
        build_ext.run(self)
        build_lib = self.build_lib
        pattern = os.path.join(build_lib, "pyfastutil", "__pyfastutil.*")
        outFiles = glob.glob(pattern)

        if outFiles:
            source_file = outFiles[0]
            target_filename = "__pyfastutil" + (".pyd" if IS_WINDOWS else ".so")
            target_file = os.path.join("./pyfastutil", target_filename)
            shutil.copy(source_file, target_file)
            shutil.move(source_file, os.path.join(build_lib, "pyfastutil", target_filename))
            print(f"File copied and renamed from {source_file} to {target_file}")
        else:
            print("No files found matching the pattern")


if __name__ == "__main__":
    sources = []
    for root, dirs, files in os.walk("./pyfastutil/src"):
        for file in files:
            if file.endswith(".cpp") or file.endswith(".c"):
                sources.append(os.path.join(root, file))

    modules = [
        Extension(
            name="pyfastutil.__pyfastutil",
            sources=sources,
            language="c++",
            extra_compile_args=EXTRA_COMPILE_ARG,
            extra_link_args=EXTRA_LINK_ARG,
            include_dirs=["./pyfastutil/src"]
        )
    ]

    setup(
        name="__pyfastutil",
        description="C++ implementation of PyFastUtil.",
        ext_modules=modules,
        cmdclass={
            "clean": CleanCommand,
            "build_ext": CustomBuildExtCommand,
        },
        package_data={
            "pyfastutil": ["src/**/*.h"],
        }
    )
