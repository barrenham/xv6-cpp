# `kernel/support/placement_new.hpp`

开发职责：freestanding 运行时、字节操作和诊断输出。

源码：[打开实际文件](../../../../kernel/support/placement_new.hpp)。文档不会加入源码或生成额外 C++ 声明。

## 函数导航

- [`operator new`](placement_new.hpp.md#fn-e1f211542b64adce)

<a id="fn-e1f211542b64adce"></a>

## `operator new`

```doxygen
@fn inline void *operator new(__SIZE_TYPE__, void *storage) noexcept
@brief 在调用者提供的地址上进行 placement 构造，不调用内核堆分配。
@param storage 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@return 各分支实际返回：`storage`。应结合上面的条件检查区分正常值、空值和错误值。
```

## 格式、边界与使用约定

placement new 只返回调用者提供的 storage，不分配物理页或堆内存。调用方必须提供足够大小和正确对齐的存储，并负责之后的对象析构和底层页面归还。
