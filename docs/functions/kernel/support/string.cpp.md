# `kernel/support/string.cpp`

开发职责：freestanding 运行时、字节操作和诊断输出。

源码：[打开实际文件](../../../../kernel/support/string.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.runtime`。

## 函数导航

- [`xv::runtime::compare_bytes`](string.cpp.md#fn-d746938e9fff88f5)
- [`xv::runtime::compare_text`](string.cpp.md#fn-fb4f1f87bb6c9655)
- [`xv::runtime::copy_fixed`](string.cpp.md#fn-676164e45c23a5a8)
- [`xv::runtime::copy_string`](string.cpp.md#fn-c343ffb82a61cdc0)
- [`xv::runtime::fill`](string.cpp.md#fn-ef93f4a780c32fda)
- [`xv::runtime::length`](string.cpp.md#fn-a83d5710b35c41e9)
- [`xv::runtime::move`](string.cpp.md#fn-da7e6b0b68e76d10)

<a id="fn-ef93f4a780c32fda"></a>

## `xv::runtime::fill`

```doxygen
@fn void *xv::runtime::fill(void *dst, int c, unsigned long n)
@brief freestanding 运行时、字节操作和诊断输出：把字节区域填充为指定值。
@param dst 结果缓冲起点。
@param c 本次操作使用的 c 值。
@param n 本次处理的数量上限。
@details 本函数可见的状态更新包括：`cdst[i] = c;`。局部临时变量与对象字段应按作用域区分。
@return 各分支实际返回：`dst`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-da7e6b0b68e76d10"></a>

## `xv::runtime::move`

```doxygen
@fn void *xv::runtime::move(void *dst, const void *src, unsigned long n)
@brief freestanding 运行时、字节操作和诊断输出：移动字节区域并允许源目标重叠。
@param dst 结果缓冲起点。
@param src 输入数据起点。
@param n 本次处理的数量上限。
@details 需要特别跟踪的条件：`n == 0`；`s < d && s + n > d`；`n-- > 0`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`dst`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-d746938e9fff88f5"></a>

## `xv::runtime::compare_bytes`

```doxygen
@fn int xv::runtime::compare_bytes(const void *v1, const void *v2, unsigned long n)
@brief freestanding 运行时、字节操作和诊断输出：compare字节区域。
@param v1 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param v2 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param n 本次处理的数量上限。
@details 需要特别跟踪的条件：`n-- > 0`；`*s1 != *s2`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`*s1 - *s2`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-fb4f1f87bb6c9655"></a>

## `xv::runtime::compare_text`

```doxygen
@fn int xv::runtime::compare_text(const char *p, const char *q, unsigned long n)
@brief freestanding 运行时、字节操作和诊断输出：compare文本。
@param p 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param q 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param n 本次处理的数量上限。
@details 需要特别跟踪的条件：`n > 0 && *p && *p == *q`；`n == 0`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`0`；`(uchar)*p - (uchar)*q`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-676164e45c23a5a8"></a>

## `xv::runtime::copy_fixed`

```doxygen
@fn char *xv::runtime::copy_fixed(char *s, const char *t, int n)
@brief freestanding 运行时、字节操作和诊断输出：复制固定宽度字符串字段。
@param s 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param t 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param n 本次处理的数量上限。
@details 需要特别跟踪的条件：`n-- > 0 && (*s++ = *t++) != 0`；`n-- > 0`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`os`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-c343ffb82a61cdc0"></a>

## `xv::runtime::copy_string`

```doxygen
@fn char *xv::runtime::copy_string(char *s, const char *t, int n)
@brief freestanding 运行时、字节操作和诊断输出：在长度上限内复制以零结尾的字符串。
@param s 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param t 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param n 本次处理的数量上限。
@details 需要特别跟踪的条件：`n <= 0`；`--n > 0 && (*s++ = *t++) != 0`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`os`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-a83d5710b35c41e9"></a>

## `xv::runtime::length`

```doxygen
@fn int xv::runtime::length(const char *s)
@brief freestanding 运行时、字节操作和诊断输出：计算以零结尾字符串的长度。
@param s 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@return 各分支实际返回：`n`。应结合上面的条件检查区分正常值、空值和错误值。
```
