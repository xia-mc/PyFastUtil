<div align="center">

English | [简体中文](./README_CN.md)

# PyFastUtil

<p>
   <img src="./mascot.png" alt="Project Mascot" width="500">
</p>

## Make Python Fast Again

[![License](https://img.shields.io/badge/license-Apache%202.0-blue.svg)](LICENSE)
[![Issues](https://img.shields.io/github/issues/xia-mc/PyFastUtil)](https://img.shields.io/github/issues/xia-mc/PyFastUtil)
![Version](https://img.shields.io/badge/CPython-3.9_and_later-blue)
[![Build](https://img.shields.io/github/actions/workflow/status/xia-mc/PyFastUtil/python-package.yml)](https://github.com/xia-mc/PyFastUtil/actions)

</div>

## Introduction

**PyFastUtil** is a high-performance utility library for Python, inspired by Java's [FastUtil](https://fastutil.di.unimi.it/) library. However, **PyFastUtil is NOT a Python binding for FastUtil**. It is a library built from the ground up, designed to bring C-like efficiency and functionality to CPython.

### Features

- Implements all corresponding Python data structure interfaces while significantly improving performance through targeted optimizations. Users can choose the most suitable data structure type based on their needs.
- Fully implemented in C/C++ with hardware-level optimizations such as SIMD, aiming to maximize the performance of data structures.
- Provides efficient Python bindings for some C APIs, allowing advanced users to perform "unsafe" low-level operations.
- Enables inline assembly, allowing advanced users to dynamically generate, invoke, and destroy C functions at runtime.

### Benchmark

> **Note**: For extremely fast \(O(1)\) operations (e.g., `pop`, `extend`), PyFastUtil's performance may be slightly inferior to Python's native implementation due to the unavoidable overhead of calling C extensions from CPython. We are actively working to optimize this.

> CPU: AMD Ryzen 7 5700G (AVX2)
> 
> Windows 11 23H2, Python 3.12, MSVC 19.41.34120

#### Type-Specialized List (e.g., `IntArrayList`)

```text
Preparing data...
---Python list & IntArrayList Benchmark---
Batch size: 10000
Repeat: 3

Python list init time: 0.02 ms
PyFastUtil IntArrayList init time: 0.08 ms
PyFastUtil speed of Python list (init): 29.788 %

Python list copy time: 0.02 ms
PyFastUtil IntArrayList copy time: 0.00 ms
PyFastUtil speed of Python list (copy): 766.102 %

Python list to_python time: 0.02 ms
PyFastUtil IntArrayList to_python time: 0.27 ms
PyFastUtil speed of Python list (to_python): 5.821 %

Python list sequential_access time: 0.00 ms
PyFastUtil IntArrayList sequential_access time: 0.00 ms
PyFastUtil speed of Python list (sequential_access): 126.667 %

Python list random_access time: 0.39 ms
PyFastUtil IntArrayList random_access time: 0.62 ms
PyFastUtil speed of Python list (random_access): 62.423 %

Python list sort time: 1.29 ms
PyFastUtil IntArrayList sort time: 0.04 ms
PyFastUtil speed of Python list (sort): 3344.483 %

Python list append time: 0.23 ms
PyFastUtil IntArrayList append time: 0.32 ms
PyFastUtil speed of Python list (append): 72.517 %

Python list insert time: 70.74 ms
PyFastUtil IntArrayList insert time: 10.93 ms
PyFastUtil speed of Python list (insert): 647.349 %

Python list pop time: 0.29 ms
PyFastUtil IntArrayList pop time: 0.35 ms
PyFastUtil speed of Python list (pop): 85.143 %

Python list remove time: 5.50 ms
PyFastUtil IntArrayList remove time: 2.54 ms
PyFastUtil speed of Python list (remove): 216.749 %

Python list contains time: 258.02 ms
PyFastUtil IntArrayList contains time: 2.61 ms
PyFastUtil speed of Python list (contains): 9896.599 %

Python list index time: 438.12 ms
PyFastUtil IntArrayList index time: 3.77 ms
PyFastUtil speed of Python list (index): 11618.038 %

Python list extend time: 0.08 ms
PyFastUtil IntArrayList extend time: 0.14 ms
PyFastUtil speed of Python list (extend): 58.433 %

Python list reverse time: 0.00 ms
PyFastUtil IntArrayList reverse time: 0.00 ms
PyFastUtil speed of Python list (reverse): 378.948 %


Avg speed of PyFastUtil compared to Python list: 1950.647 %
```

#### Generic Type List (e.g., `ObjectArrayList`)

```text
Preparing data...
---Python list & ObjectArrayList Benchmark---
Batch size: 10000
Repeat: 3

Python list init time: 0.06 ms
PyFastUtil ObjectArrayList init time: 0.05 ms
PyFastUtil speed of Python list (init): 129.484 %

Python list copy time: 0.07 ms
PyFastUtil ObjectArrayList copy time: 0.03 ms
PyFastUtil speed of Python list (copy): 268.376 %

Python list to_python time: 0.03 ms
PyFastUtil ObjectArrayList to_python time: 0.03 ms
PyFastUtil speed of Python list (to_python): 99.325 %

Python list sequential_access time: 0.00 ms
PyFastUtil ObjectArrayList sequential_access time: 0.00 ms
PyFastUtil speed of Python list (sequential_access): 139.130 %

Python list random_access time: 0.22 ms
PyFastUtil ObjectArrayList random_access time: 0.31 ms
PyFastUtil speed of Python list (random_access): 71.053 %

Python list sort time: 1039.91 ms
PyFastUtil ObjectArrayList sort time: 1003.55 ms
PyFastUtil speed of Python list (sort): 103.623 %

Python list append time: 0.27 ms
PyFastUtil ObjectArrayList append time: 0.33 ms
PyFastUtil speed of Python list (append): 81.091 %

Python list insert time: 72.43 ms
PyFastUtil ObjectArrayList insert time: 20.82 ms
PyFastUtil speed of Python list (insert): 347.893 %

Python list pop time: 0.36 ms
PyFastUtil ObjectArrayList pop time: 0.27 ms
PyFastUtil speed of Python list (pop): 130.323 %

Python list remove time: 5.17 ms
PyFastUtil ObjectArrayList remove time: 5.17 ms
PyFastUtil speed of Python list (remove): 100.102 %

Python list contains time: 665.64 ms
PyFastUtil ObjectArrayList contains time: 3.77 ms
PyFastUtil speed of Python list (contains): 17634.553 %

Python list index time: 985.46 ms
PyFastUtil ObjectArrayList index time: 6.80 ms
PyFastUtil speed of Python list (index): 14501.919 %

Python list extend time: 0.08 ms
PyFastUtil ObjectArrayList extend time: 0.12 ms
PyFastUtil speed of Python list (extend): 73.264 %

Python list reverse time: 0.00 ms
PyFastUtil ObjectArrayList reverse time: 0.00 ms
PyFastUtil speed of Python list (reverse): 106.000 %


Avg speed of PyFastUtil compared to Python list: 2413.296 %
```

## Installation

### Install PyFastUtil from PyPI:
```shell
pip install PyFastUtil
```

### Or, build from source

If you'd like to build the project from source, follow these steps:

1. Clone the repository:
    ```shell
    git clone https://github.com/yourusername/PyFastUtil.git
    cd PyFastUtil
    ```

2. Build the project:
    - On **Windows**:
      ```shell
      ./build.cmd
      ```
    - On **Linux/macOS**:
      ```shell
      ./build.sh
      ```

## License

This project is licensed under the **Apache License 2.0**. For more details, see the [LICENSE](LICENSE) file.

## Roadmap

- [ ] Implement `int`, `float`, and `double` ArrayList and LinkedList.
- [x] Add Numpy support.
- [x] Provide bindings for SIMD utility functions.
- [x] Provide raw AVX512 bindings.
- [x] Perform comprehensive testing and benchmarking.
- [x] Publish to PyPI.

## Contribution

Contributions are welcome! Feel free to submit issues or pull requests. Please note that the project is in its early stages, and we greatly appreciate any feedback or suggestions.

## Acknowledgements

This project is partially based on the following excellent open-source projects:

- [CPython](https://github.com/python/cpython): The official implementation of the Python interpreter, licensed under the [Python Software Foundation License](https://docs.python.org/3/license.html).
- [C++ Standard Library (STL)](https://en.cppreference.com/w/cpp): The standard library for C++, providing essential data structures, algorithms, and utilities, licensed under the [ISO C++ Standard](https://isocpp.org/).
- [cpp-TimSort](https://github.com/timsort/cpp-TimSort): A C++ implementation of the TimSort algorithm, licensed under the [MIT License](https://github.com/timsort/cpp-TimSort/blob/master/LICENSE).
- [ankerl::unordered_dense](https://github.com/martinus/unordered_dense): A modern, high-performance, low-memory hash table implementation in C++, licensed under the [MIT License](https://github.com/martinus/unordered_dense/blob/main/LICENSE).
- [qReverse](https://github.com/Wunkolo/qreverse): A high-performance, architecture-optimized array reversal algorithm, released under the [MIT License](https://github.com/martinus/unordered_dense/blob/main/LICENSE).

Special thanks to the contributors of these open-source projects!

The mascot image of the project was created by the artist [kokola](https://x.com/kokola10032) and is used with permission. You can view the original artwork on [X](https://x.com/kokola10032/status/1812480707643506704).
