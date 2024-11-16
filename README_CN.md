<div align="center">

[English](./README.md) | 简体中文

# PyFastUtil

[//]: # (  <p>)

[//]: # (    <!--suppress CheckImageSize -->)

[//]: # (      <img src="./mascot.png" alt="Project Mascot" width="320">)

[//]: # (  </p>)

## 让 Python 更快一点

[![License](https://img.shields.io/badge/license-Apache%202.0-blue.svg)](LICENSE)
[![Issues](https://img.shields.io/github/issues/xia-mc/PyFastUtil)](https://img.shields.io/github/issues/Nova-Committee/CheatDetector)
[![Build](https://github.com/xia-mc/PyFastUtil/actions/workflows/python-package.yml/badge.svg)](https://github.com/xia-mc/PyFastUtil/actions)

</div>

## 简介

**PyFastUtil** 是一个为 Python 提供的高性能工具库，灵感来源于 Java 中的 [FastUtil](https://fastutil.di.unimi.it/) 库。然而，**PyFastUtil 并不是任何现有库的 Python 绑定**，而是一个**从零开始重新实现**的库，旨在为 Python 带来与 Java FastUtil 相同的效率和功能。

> **注意**: PyFastUtil 仍处于**早期开发阶段**。代码库正在积极开发中，尚未准备好用于生产环境。我们正在努力使其功能齐全并经过广泛测试，之后会发布到 PyPI。

### 功能

- 在实现所有对应 Python 数据结构接口的同时，通过针对性优化大幅提升性能，并允许用户根据需求选择最合适的数据结构类型。
- 完全使用 C/C++ 实现，并通过 SIMD 等硬件针对性优化，旨在最大化提升数据结构的性能。
- 提供一些高效的 C API Python 绑定，允许高级用户执行一些“不安全”的底层操作。

## 安装

目前，**PyFastUtil** 还未发布到 PyPI。您需要**克隆仓库**并从源代码进行构建。

### 从源代码构建

1. 克隆仓库：
    ```bash
    git clone https://github.com/yourusername/PyFastUtil.git
    cd PyFastUtil
    ```

2. 构建项目：
    - 在 **Windows** 上：
      ```bash
      build.cmd
      ```
    - 在 **Linux/macOS** 上：
      ```bash
      ./build.sh
      ```

> **注意**: 项目功能齐全并经过广泛测试后，将发布到 PyPI。

## 许可证

本项目使用 **Apache License 2.0** 进行开源。有关详细信息，请参阅 [LICENSE](LICENSE) 文件。

## 开发路线图

- [ ] 实现核心数据结构（例如，快速列表、映射、集合）。
- [ ] 提供 SIMD 和内联汇编的 Python 绑定。
- [ ] 进行全面的测试和基准测试。
- [ ] 发布到 PyPI。

## 贡献

欢迎贡献！请随时提交问题或拉取请求。请注意，项目处于早期阶段，我们非常感谢任何反馈或建议。
