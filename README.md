<div align="center">

English | [简体中文](./README_CN.md)

# PyFastUtil

<p>
   <img src="./mascot.png" alt="Project Mascot" width="500">
</p>

## Make Python Fast Again

[![License](https://img.shields.io/badge/license-Apache%202.0-blue.svg)](LICENSE)
[![Issues](https://img.shields.io/github/issues/xia-mc/PyFastUtil)](https://img.shields.io/github/issues/xia-mc/PyFastUtil)
![Version](https://img.shields.io/badge/CPython-3.9_or_later-blue)
[![Build](https://img.shields.io/github/actions/workflow/status/xia-mc/PyFastUtil/python-package.yml)](https://github.com/xia-mc/PyFastUtil/actions)

</div>

## Introduction

**PyFastUtil** is a high-performance utility library for Python, inspired by the popular [FastUtil](https://fastutil.di.unimi.it/) library in Java. However, **PyFastUtil is not a Python binding of any existing library**, but a **complete re-implementation** from scratch, designed to bring the same efficiency and functionality to Python.

> **Note**: PyFastUtil is still in its **early development phase**. The codebase is under active development and is not yet ready for production use. 
> We are working hard to make it feature-complete and thoroughly tested before releasing it on PyPI.

### Features

- Implements all corresponding Python data structure interfaces, while providing significant performance improvements through targeted optimizations. Users can choose the most suitable data structure for their specific needs.
- Fully implemented in C/C++, leveraging hardware-specific optimizations such as SIMD to maximize the performance of data structures.
- Offers efficient Python bindings for certain C APIs, allowing advanced users to perform low-level, "unsafe" operations when needed.

### Performance Benchmark

> **Note**: For some very fast O(1) operations (such as `pop` and `extend`), PyFastUtil may perform slightly worse than Python's native implementation due to the non-negligible overhead of CPython's C extension calls. We are working on optimizing this.

> CPU: AMD Ryzen 7 5700G (AVX2)
> 
> Windows 11 23H2, Python 3.12, MSVC 19.41.34120

#### Specialized List Benchmark (e.g., `IntArrayList`)

```text
---Python list & IntArrayList Benchmark---
Batch size: 10000
Repeat: 3

Python list sort time: 1.31 ms
PyFastUtil IntArrayList sort time: 0.03 ms
PyFastUtil speed of Python list (sort): 4139.240 %

Python list append time: 0.44 ms
PyFastUtil IntArrayList append time: 0.43 ms
PyFastUtil speed of Python list (append): 102.314 %

Python list insert time: 71.75 ms
PyFastUtil IntArrayList insert time: 10.68 ms
PyFastUtil speed of Python list (insert): 671.948 %

Python list pop time: 0.34 ms
PyFastUtil IntArrayList pop time: 0.40 ms
PyFastUtil speed of Python list (pop): 85.341 %

Python list remove time: 5.13 ms
PyFastUtil IntArrayList remove time: 2.55 ms
PyFastUtil speed of Python list (remove): 201.138 %

Python list contains time: 536.74 ms
PyFastUtil IntArrayList contains time: 15.29 ms
PyFastUtil speed of Python list (contains): 3511.205 %

Python list index time: 414.75 ms
PyFastUtil IntArrayList index time: 3.85 ms
PyFastUtil speed of Python list (index): 10764.189 %

Python list extend time: 0.09 ms
PyFastUtil IntArrayList extend time: 0.20 ms
PyFastUtil speed of Python list (extend): 44.228 %


Avg speed of PyFastUtil compared to Python list: 2439.950 %
```

#### Generic List Benchmark (e.g., `ObjectArrayList`)

```text
---Python list & ObjectArrayList Benchmark---
Batch size: 10000
Repeat: 3

Python list sort time: 125.56 ms
PyFastUtil ObjectArrayList sort time: 116.61 ms
PyFastUtil speed of Python list (sort): 107.668 %

Python list append time: 0.30 ms
PyFastUtil ObjectArrayList append time: 0.35 ms
PyFastUtil speed of Python list (append): 86.724 %

Python list insert time: 81.55 ms
PyFastUtil ObjectArrayList insert time: 23.95 ms
PyFastUtil speed of Python list (insert): 340.471 %

Python list pop time: 0.44 ms
PyFastUtil ObjectArrayList pop time: 0.32 ms
PyFastUtil speed of Python list (pop): 139.226 %

Python list remove time: 5.25 ms
PyFastUtil ObjectArrayList remove time: 5.14 ms
PyFastUtil speed of Python list (remove): 102.170 %

Python list contains time: 1140.28 ms
PyFastUtil ObjectArrayList contains time: 16.37 ms
PyFastUtil speed of Python list (contains): 6966.576 %

Python list index time: 866.88 ms
PyFastUtil ObjectArrayList index time: 6.77 ms
PyFastUtil speed of Python list (index): 12803.132 %

Python list extend time: 0.12 ms
PyFastUtil ObjectArrayList extend time: 0.15 ms
PyFastUtil speed of Python list (extend): 80.975 %


Avg speed of PyFastUtil compared to Python list: 2578.368 %
```

## Installation

Currently, **PyFastUtil** is not available on PyPI. You will need to **clone the repository** and build it from the source code.

### Build from source

1. Clone the repository:
    ```bash
    git clone https://github.com/yourusername/PyFastUtil.git
    cd PyFastUtil
    ```

2. Build the project:
    - On **Windows**:
      ```bash
      build.cmd
      ```
    - On **Linux/macOS**:
      ```bash
      ./build.sh
      ```

> **Note**: Future releases will be available on PyPI once the project is feature-complete and well-tested.

## License

This project is licensed under the **Apache License 2.0**. See the [LICENSE](LICENSE) file for more details.

## Roadmap

- [ ] Implement core data structures (e.g., fast lists, maps, sets).
- [x] Numpy support.
- [ ] Provide SIMD and inline assembly bindings for Python.
- [ ] Thorough testing and benchmarking.
- [ ] PyPI release.

## Contributing

Contributions are welcome! Feel free to submit issues or pull requests. Please note that the project is in an early stage, and we appreciate any feedback or suggestions.

## Acknowledgements

This project includes code from the following amazing open-source projects:

- [CPython](https://github.com/python/cpython): The official Python interpreter, licensed under the [Python Software Foundation License](https://docs.python.org/3/license.html).
- [cpp-TimSort](https://github.com/timsort/cpp-TimSort): A C++ implementation of the TimSort algorithm, licensed under the [MIT License](https://github.com/timsort/cpp-TimSort/blob/master/LICENSE).

Special thanks to the contributors of these open-source projects!

The "mascot" image is provided by the artist [kokola](https://x.com/kokola10032) and is used with permission. You can find the original image on [X](https://x.com/kokola10032/status/1812480707643506704).
