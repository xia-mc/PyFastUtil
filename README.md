<div align="center">

English | [简体中文](./README_CN.md)

# PyFastUtil

[//]: # (  <p>)

[//]: # (    <!--suppress CheckImageSize -->)

[//]: # (      <img src="./mascot.png" alt="Project Mascot" width="320">)

[//]: # (  </p>)

## Make Python Fast Again

[![License](https://img.shields.io/badge/license-Apache%202.0-blue.svg)](LICENSE)
[![Issues](https://img.shields.io/github/issues/xia-mc/PyFastUtil)](https://img.shields.io/github/issues/Nova-Committee/CheatDetector)
[![Build](https://github.com/xia-mc/PyFastUtil/actions/workflows/python-package.yml/badge.svg)](https://github.com/xia-mc/PyFastUtil/actions)

</div>

## Introduction

**PyFastUtil** is a high-performance utility library for Python, inspired by the popular [FastUtil](https://fastutil.di.unimi.it/) library in Java. However, **PyFastUtil is not a Python binding of any existing library**, but a **complete re-implementation** from scratch, designed to bring the same efficiency and functionality to Python.

> **Note**: PyFastUtil is still in its **early development phase**. The codebase is under active development and is not yet ready for production use. We are working hard to make it feature-complete and thoroughly tested before releasing it on PyPI.

### Features

- Implements all corresponding Python data structure interfaces, while providing significant performance improvements through targeted optimizations. Users can choose the most suitable data structure for their specific needs.
- Fully implemented in C/C++, leveraging hardware-specific optimizations such as SIMD to maximize the performance of data structures.
- Offers efficient Python bindings for certain C APIs, allowing advanced users to perform low-level, "unsafe" operations when needed.

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
