<div align="center">

[English](./README.md) | 简体中文

# PyFastUtil

<p>
   <img src="./mascot.png" alt="Project Mascot" width="500">
</p>

## 让 Python 再快一点

[![License](https://img.shields.io/badge/license-Apache%202.0-blue.svg)](LICENSE)
[![Issues](https://img.shields.io/github/issues/xia-mc/PyFastUtil)](https://img.shields.io/github/issues/xia-mc/PyFastUtil)
![Version](https://img.shields.io/badge/CPython-3.9_and_later-blue)
[![Build](https://img.shields.io/github/actions/workflow/status/xia-mc/PyFastUtil/python-package.yml)](https://github.com/xia-mc/PyFastUtil/actions)

</div>

## 简介

**PyFastUtil** 是一个为 Python 提供的高性能工具库，灵感来源于 Java 中的 [FastUtil](https://fastutil.di.unimi.it/) 库。然而，**PyFastUtil 并不是 FastUtil 的 Python 绑定**，而是一个从零开始重新实现的库，旨在为 CPython 带来与 C 般的效率和功能。

### 功能

- 在实现所有对应 Python 数据结构接口的同时，通过针对性优化大幅提升性能，并允许用户根据需求选择最合适的数据结构类型。
- 完全使用 C/C++ 实现，并通过 SIMD 等硬件针对性优化，旨在最大化提升数据结构的性能。
- 提供一些高效的 C API Python 绑定，允许高级用户执行一些“不安全”的底层操作。
- 提供内联汇编的能力，允许高级用户在运行时动态生成/调用/销毁C函数。

### 性能测试

> **注意**: 对于一些非常快速的 O(1) 操作（如 `pop`、`extend`），PyFastUtil 的性能可能会略逊于 Python 原生实现。这是由于 CPython 调用 C 扩展时的不可忽视的开销所致。我们正在努力优化这一点。

> CPU: AMD Ryzen 7 5700G (AVX2)
> 
> Windows 11 23H2, Python 3.12, MSVC 19.41.34120

#### 针对类型特化的列表（以 `IntArrayList` 为例）

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

#### 针对存储通用类型的列表（以 `ObjectArrayList` 为例）

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

## 安装

### 从 PyPI 安装 PyFastUtil:
```shell
pip install PyFastUtil
```

### 或者，从源代码构建

如果您希望从源代码构建项目，请按照以下步骤操作：

1. 克隆仓库：
    ```shell
    git clone https://github.com/yourusername/PyFastUtil.git
    cd PyFastUtil
    ```

2. 构建项目：
    - 在 **Windows** 上：
      ```shell
      ./build.cmd
      ```
    - 在 **Linux/macOS** 上：
      ```shell
      ./build.sh
      ```

> **注意**: 项目功能齐全并经过广泛测试后，将正式发布到 PyPI。

## 许可证

本项目使用 **Apache License 2.0** 进行开源。有关详细信息，请参阅 [LICENSE](LICENSE) 文件。

## 开发路线图

- [ ] 实现int, float, double的ArrayList, LinkedList。
- [x] Numpy 支持。
- [x] 提供 SIMD 封装函数的绑定。
- [x] 提供 AVX512 的原始底层绑定。
- [x] 进行全面的测试和基准测试。
- [x] 发布到 PyPI。

## 贡献

欢迎贡献！请随时提交问题或拉取请求。请注意，项目处于早期阶段，我们非常感谢任何反馈或建议。

## 感谢

本项目部分代码基于以下优秀的开源项目：

- [CPython](https://github.com/python/cpython): Python 官方解释器的实现，遵循 [Python Software Foundation License](https://docs.python.org/3/license.html)。
- [C++ 标准库 (STL)](https://en.cppreference.com/w/cpp): 提供了基础数据结构、算法和工具的 C++ 标准库，遵循 [ISO C++ 标准](https://isocpp.org/)。
- [cpp-TimSort](https://github.com/timsort/cpp-TimSort): C++ 实现的 TimSort 算法，遵循 [MIT License](https://github.com/timsort/cpp-TimSort/blob/master/LICENSE)。
- [ankerl::unordered_dense](https://github.com/martinus/unordered_dense): 一个现代 C++ 的高性能、低内存占用的哈希表实现，遵循 [MIT License](https://github.com/martinus/unordered_dense/blob/main/LICENSE)。
- [qReverse](https://github.com/Wunkolo/qreverse)：一个高性能、架构优化的数组反转算法，基于 [MIT 许可证](https://github.com/martinus/unordered_dense/blob/main/LICENSE) 发布。

特别感谢这些开源项目的贡献者们！

“看板娘”图像由画师 [kokola](https://x.com/kokola10032) 提供，并已获得使用许可。您可以在 [X](https://x.com/kokola10032/status/1812480707643506704) 上查看原图。
