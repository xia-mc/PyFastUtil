<div align="center">

[English](./README.md) | 简体中文

# PyFastUtil

> 你可以在 [这里](https://x.com/kokola10032/status/1812480707643506704) 查看图片.

[//]: # (<p>)

[//]: # (   <img src="https://pbs.twimg.com/media/GSc5lbxWMAAOM9l?format=png" alt="Project Mascot" width="400">)

[//]: # (</p>)

[//]: # ()
[//]: # (Image author: [Twitter@kokola10032]&#40;https://x.com/kokola10032&#41;)

## 让 Python 更快一点

[![License](https://img.shields.io/badge/license-Apache%202.0-blue.svg)](LICENSE)
[![Issues](https://img.shields.io/github/issues/xia-mc/PyFastUtil)](https://img.shields.io/github/issues/Nova-Committee/CheatDetector)
[![Build](https://github.com/xia-mc/PyFastUtil/actions/workflows/python-package.yml/badge.svg)](https://github.com/xia-mc/PyFastUtil/actions)

</div>

## 简介

**PyFastUtil** 是一个为 Python 提供的高性能工具库，灵感来源于 Java 中的 [FastUtil](https://fastutil.di.unimi.it/) 库。然而，**PyFastUtil 并不是任何现有库的 Python 绑定**，而是一个**从零开始重新实现**的库，旨在为 Python 带来与 Java FastUtil 相同的效率和功能。

> **注意**: PyFastUtil 仍处于**早期开发阶段**。代码库正在积极开发中，尚未准备好用于生产环境。
> 我们正在努力使其功能齐全并经过广泛测试，之后会发布到 PyPI。

### 功能

- 在实现所有对应 Python 数据结构接口的同时，通过针对性优化大幅提升性能，并允许用户根据需求选择最合适的数据结构类型。
- 完全使用 C/C++ 实现，并通过 SIMD 等硬件针对性优化，旨在最大化提升数据结构的性能。
- 提供一些高效的 C API Python 绑定，允许高级用户执行一些“不安全”的底层操作。

### 性能测试

> **注意**: 对于一些非常快速的 O(1) 操作（如 `pop`、`extend`），PyFastUtil 的性能可能会略逊于 Python 原生实现。这是由于 CPython 调用 C 扩展时的不可忽视的开销所致。我们正在努力优化这一点。

#### 针对类型特化的列表（以 `IntArrayList` 为例）

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

#### 针对存储通用类型的列表（以 `ObjectArrayList` 为例）

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
