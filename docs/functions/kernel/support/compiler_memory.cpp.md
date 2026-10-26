# `kernel/support/compiler_memory.cpp`

开发职责：freestanding 运行时、字节操作和诊断输出。

源码：[打开实际文件](../../../../kernel/support/compiler_memory.cpp)。文档不会加入源码或生成额外 C++ 声明。

直接模块依赖：`xv.runtime`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`memcmp`](compiler_memory.cpp.md#fn-6d739c895b768de9)
- [`memcpy`](compiler_memory.cpp.md#fn-9fb50521ee51f448)
- [`memmove`](compiler_memory.cpp.md#fn-45c4f9eae474b83c)
- [`memset`](compiler_memory.cpp.md#fn-335fe7788debe05d)

<a id="fn-335fe7788debe05d"></a>

## `memset`

```doxygen
@fn void *memset(void *destination, int value, unsigned long count)
@brief freestanding 运行时、字节操作和诊断输出：读取或计算 `xv::runtime::fill(destination, value, count)`，返回当前查询结果。
@param destination 接收结果的缓冲或目标对象。
@param value 本次操作使用的 value 值。
@param count 本次操作的数量上限。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::runtime::fill`。
@return 各分支实际返回：`xv::runtime::fill(destination, value, count)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-45c4f9eae474b83c"></a>

## `memmove`

```doxygen
@fn void *memmove(void *destination, const void *source, unsigned long count)
@brief freestanding 运行时、字节操作和诊断输出：读取或计算 `xv::runtime::move(destination, source, count)`，返回当前查询结果。
@param destination 接收结果的缓冲或目标对象。
@param source 输入数据位置或源对象。
@param count 本次操作的数量上限。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::runtime::move`。
@return 各分支实际返回：`xv::runtime::move(destination, source, count)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-9fb50521ee51f448"></a>

## `memcpy`

```doxygen
@fn void *memcpy(void *destination, const void *source, unsigned long count)
@brief freestanding 运行时、字节操作和诊断输出：读取或计算 `xv::runtime::move(destination, source, count)`，返回当前查询结果。
@param destination 接收结果的缓冲或目标对象。
@param source 输入数据位置或源对象。
@param count 本次操作的数量上限。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::runtime::move`。
@return 各分支实际返回：`xv::runtime::move(destination, source, count)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-6d739c895b768de9"></a>

## `memcmp`

```doxygen
@fn int memcmp(const void *first, const void *second, unsigned long count)
@brief freestanding 运行时、字节操作和诊断输出：读取或计算 `xv::runtime::compare_bytes(first, second, count)`，返回当前查询结果。
@param first 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param second 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param count 本次操作的数量上限。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::runtime::compare_bytes`。
@return 各分支实际返回：`xv::runtime::compare_bytes(first, second, count)`。应结合上面的条件检查区分正常值、空值和错误值。
```
