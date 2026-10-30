# `kernel/process/storage/kernel_platform.cpp`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../kernel/process/storage/kernel_platform.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process`。

直接模块依赖：`:storage_platform`, `xv.memory`, `xv.print`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::process::detail::allocate_kernel_stack`](kernel_platform.cpp.md#fn-0c73cf7a89069b02)
- [`xv::process::detail::allocate_process_page`](kernel_platform.cpp.md#fn-e1bcdb7d80c1c4c6)
- [`xv::process::detail::free_kernel_stack`](kernel_platform.cpp.md#fn-0ccd821dd7af8126)
- [`xv::process::detail::free_process_page`](kernel_platform.cpp.md#fn-470a48c1e3751448)
- [`xv::process::detail::scheduling_time`](kernel_platform.cpp.md#fn-4fba22f386559889)
- [`xv::process::detail::storage_fail`](kernel_platform.cpp.md#fn-81d9f378847f3b25)

- [`xv::process::detail::initialize_process_objects`](kernel_platform.cpp.md#fn-7d233fbcfe20643c)
- [`xv::process::detail::allocate_process_object`](kernel_platform.cpp.md#fn-caa4a1fb931599c7)
- [`xv::process::detail::free_process_object`](kernel_platform.cpp.md#fn-9ac05fbc453dc8b6)

<a id="fn-e1bcdb7d80c1c4c6"></a>

## `xv::process::detail::allocate_process_page`

```doxygen
@fn void *xv::process::detail::allocate_process_page()
@brief 进程对象生命周期、组身份、调度、等待与信号：读取或计算 `xv::memory::allocator().allocate()`，返回当前查询结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::memory::allocator` → `allocate`。
@return 各分支实际返回：`xv::memory::allocator().allocate()`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-470a48c1e3751448"></a>

## `xv::process::detail::free_process_page`

```doxygen
@fn void xv::process::detail::free_process_page(void *page)
@brief 进程对象生命周期、组身份、调度、等待与信号：释放进程页。
@param page 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::memory::allocator` → `deallocate`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-0c73cf7a89069b02"></a>

## `xv::process::detail::allocate_kernel_stack`

```doxygen
@fn unsigned long xv::process::detail::allocate_kernel_stack()
@brief 取得一个按需增长的虚拟内核栈。
@details 委托内存页表服务独立保留区域与初始物理页；返回栈底，零表示没有空闲槽或内存不足，不再依赖物理页编号计算别名。
```

<a id="fn-0ccd821dd7af8126"></a>

## `xv::process::detail::free_kernel_stack`

```doxygen
@fn void xv::process::detail::free_kernel_stack(unsigned long address)
@brief 回收已停止进程的按需栈区域。
@details 委托页表服务清映射、归还全部已提交帧与叶表，释放虚拟槽；调用时目标进程不能仍在该栈上执行。
```

<a id="fn-81d9f378847f3b25"></a>

## `xv::process::detail::storage_fail`

```doxygen
@fn void xv::process::detail::storage_fail(const char *message)
@brief 进程对象生命周期、组身份、调度、等待与信号：storage失败处理。
@param message 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::print::panic`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-4fba22f386559889"></a>

## `xv::process::detail::scheduling_time`

```doxygen
@fn unsigned long xv::process::detail::scheduling_time()
@brief 读取平台 r_time 作为调度实际运行时间基准。
@details 遵守原进程锁、就绪队列锁、Runnable 生命周期和上下文交接约定；详见可选时钟调度设计。
```

<a id="fn-7d233fbcfe20643c"></a>

## `xv::process::detail::initialize_process_objects`

```doxygen
@fn void xv::process::detail::initialize_process_objects(unsigned long bytes, unsigned long alignment)
@brief 在首次发布前准备精确尺寸进程描述符缓存，并校验后续初始化的布局一致性。
@details 原始对象的构造和析构由调用者负责；初始化在发布前完成，已发布缓存操作由自旋锁串行。布局、回收和测量范围见 docs/slab.md。
```

<a id="fn-caa4a1fb931599c7"></a>

## `xv::process::detail::allocate_process_object`

```doxygen
@fn void *xv::process::detail::allocate_process_object()
@brief 从精确尺寸 slab 分配稳定的进程描述符存储，与真正的整页等待缓冲分配分离。
@details 原始对象的构造和析构由调用者负责；初始化在发布前完成，已发布缓存操作由自旋锁串行。布局、回收和测量范围见 docs/slab.md。
```

<a id="fn-9ac05fbc453dc8b6"></a>

## `xv::process::detail::free_process_object`

```doxygen
@fn void xv::process::detail::free_process_object(void *object)
@brief 将已销毁的进程描述符归还所属精确尺寸 slab，非法归还视为内核错误。
@details 原始对象的构造和析构由调用者负责；初始化在发布前完成，已发布缓存操作由自旋锁串行。布局、回收和测量范围见 docs/slab.md。
```
