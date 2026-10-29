# `kernel/support/runtime.cppm`

开发职责：freestanding 运行时、字节操作和诊断输出。

源码：[打开实际文件](../../../../kernel/support/runtime.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.runtime`。

## 函数导航

- [`xv::runtime::array_size`](runtime.cppm.md#fn-075eab47bead8c60)
- [`xv::runtime::compare_bytes`](string.cpp.md#fn-d746938e9fff88f5)
- [`xv::runtime::compare_text`](string.cpp.md#fn-fb4f1f87bb6c9655)
- [`xv::runtime::copy_fixed`](string.cpp.md#fn-676164e45c23a5a8)
- [`xv::runtime::copy_string`](string.cpp.md#fn-c343ffb82a61cdc0)
- [`xv::runtime::fill`](string.cpp.md#fn-ef93f4a780c32fda)
- [`xv::runtime::length`](string.cpp.md#fn-a83d5710b35c41e9)
- [`xv::runtime::move`](string.cpp.md#fn-da7e6b0b68e76d10)

<a id="fn-075eab47bead8c60"></a>

## `xv::runtime::array_size`

```doxygen
@fn constexpr unsigned long xv::runtime::array_size(const T (&)[Count])
@brief freestanding 运行时、字节操作和诊断输出：在编译期取得数组元素数量。
@return 各分支实际返回：`Count`。应结合上面的条件检查区分正常值、空值和错误值。
```
