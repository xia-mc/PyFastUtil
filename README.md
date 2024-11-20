<div align="center">

English | [简体中文](./README_CN.md)

# PyFastUtil

<p>
   <img src="./mascot.png" alt="Project Mascot" width="400">
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

> CPU: AMD Ryzen 7 5700G
> 
> Windows 11 23H2, Python 3.12, MSVC 19.41.34120

#### Specialized List Benchmark (e.g., `IntArrayList`)

```text
---Python list & IntArrayList Benchmark---
Batch size: 10000
Repeat: 3

Python list sort time: 0.82 ms
PyFastUtil IntArrayList sort time: 0.03 ms
PyFastUtil speed of Python list (sort): 2346.991 %

Python list append time: 0.28 ms
PyFastUtil IntArrayList append time: 0.31 ms
PyFastUtil speed of Python list (append): 89.886 %

Python list insert time: 70.51 ms
PyFastUtil IntArrayList insert time: 10.59 ms
PyFastUtil speed of Python list (insert): 665.616 %

Python list pop time: 0.31 ms
PyFastUtil IntArrayList pop time: 0.34 ms
PyFastUtil speed of Python list (pop): 90.092 %

Python list remove time: 5.14 ms
PyFastUtil IntArrayList remove time: 2.52 ms
PyFastUtil speed of Python list (remove): 204.007 %

Python list contains time: 259.78 ms
PyFastUtil IntArrayList contains time: 12.24 ms
PyFastUtil speed of Python list (contains): 2122.882 %

Python list index time: 232.94 ms
PyFastUtil IntArrayList index time: 2.92 ms
PyFastUtil speed of Python list (index): 7964.229 %

Python list extend time: 0.09 ms
PyFastUtil IntArrayList extend time: 0.15 ms
PyFastUtil speed of Python list (extend): 59.879 %


Avg speed of PyFastUtil compared to Python list: 1692.948 %
```

#### Generic List Benchmark (e.g., `ObjectArrayList`)

```text
---Python list & ObjectArrayList Benchmark---
Batch size: 10000
Repeat: 3

Python list sort time: 82.11 ms
PyFastUtil ObjectArrayList sort time: 80.52 ms
PyFastUtil speed of Python list (sort): 101.963 %

Python list append time: 0.29 ms
PyFastUtil ObjectArrayList append time: 0.32 ms
PyFastUtil speed of Python list (append): 89.097 %

Python list insert time: 70.45 ms
PyFastUtil ObjectArrayList insert time: 20.46 ms
PyFastUtil speed of Python list (insert): 344.407 %

Python list pop time: 0.31 ms
PyFastUtil ObjectArrayList pop time: 0.28 ms
PyFastUtil speed of Python list (pop): 112.496 %

Python list remove time: 5.23 ms
PyFastUtil ObjectArrayList remove time: 5.15 ms
PyFastUtil speed of Python list (remove): 101.612 %

Python list contains time: 1074.47 ms
PyFastUtil ObjectArrayList contains time: 16.07 ms
PyFastUtil speed of Python list (contains): 6684.718 %

Python list index time: 873.44 ms
PyFastUtil ObjectArrayList index time: 6.71 ms
PyFastUtil speed of Python list (index): 13016.306 %

Python list extend time: 0.08 ms
PyFastUtil ObjectArrayList extend time: 0.11 ms
PyFastUtil speed of Python list (extend): 75.165 %


Avg speed of PyFastUtil compared to Python list: 2565.720 %
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
