# `kernel/memory/boot/kalloc.cpp`

开发职责：物理页所有权、地址范围、页表和用户内存复制。

源码：[打开实际文件](../../../../../kernel/memory/boot/kalloc.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.memory`。

直接模块依赖：`xv.print`, `xv.runtime`, `:detail`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::memory::BootInitializeFlow::discover`](kalloc.cpp.md#fn-ca094a83ada9f57d)
- [`xv::memory::BootInitializeFlow::initialize`](kalloc.cpp.md#fn-02f29380e4420738)
- [`xv::memory::BootInitializeFlow::invalid_layout`](kalloc.cpp.md#fn-92da10524e4a727d)
- [`xv::memory::BootInitializeFlow::invalid_tree`](kalloc.cpp.md#fn-eae3c99cdef2d9fd)
- [`xv::memory::BootInitializeFlow::ok`](kalloc.cpp.md#fn-b04607679bc30c19)
- [`xv::memory::BootInitializeFlow::outside`](kalloc.cpp.md#fn-1f3f05c5ba65af6c)
- [`xv::memory::BootInitializeFlow::report`](kalloc.cpp.md#fn-e94df4064c4365f5)
- [`xv::memory::BootInitializeFlow::resident`](kalloc.cpp.md#fn-c14473c4e8d3b353)
- [`xv::memory::boot_initialize`](kalloc.cpp.md#fn-df1a8ede82b392e0)
- [`xv::memory::configured_allocator_kind`](kalloc.cpp.md#fn-d516d0e9e55b7d40)
- [`xv::memory::detail::fail`](kalloc.cpp.md#fn-b1d0993dea3ca642)
- [`xv::memory::detail::fill`](kalloc.cpp.md#fn-e2dab6f4860894f9)

<a id="fn-b1d0993dea3ca642"></a>

## `xv::memory::detail::fail`

```doxygen
@fn void xv::memory::detail::fail(const char *message)
@brief 物理页所有权、地址范围、页表和用户内存复制：报告内核不变量错误并停止正常执行。
@param message 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::print::panic`。
@return 不返回正常调用点；后续执行流由异常终止、调度或特权级切换接管。
```

<a id="fn-e2dab6f4860894f9"></a>

## `xv::memory::detail::fill`

```doxygen
@fn void xv::memory::detail::fill(void *address, int value, MemorySize length)
@brief 物理页所有权、地址范围、页表和用户内存复制：把字节区域填充为指定值。
@param address 地址参数；用户地址必须通过内存复制/映射校验路径访问。
@param value 本次操作使用的 value 值。
@param length 本次操作使用的 length 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::runtime::fill`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-d516d0e9e55b7d40"></a>

## `xv::memory::configured_allocator_kind`

```doxygen
@fn AllocatorKind xv::memory::configured_allocator_kind()
@brief 物理页所有权、地址范围、页表和用户内存复制：读取或计算 `AllocatorKind::FreeList`，返回当前查询结果。
@return 各分支实际返回：`AllocatorKind::FreeList`；`AllocatorKind::Buddy`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-df1a8ede82b392e0"></a>

## `xv::memory::boot_initialize`

```doxygen
@fn void xv::memory::boot_initialize(AllocatorKind kind, PhysicalAddress device_tree)
@brief 物理页所有权、地址范围、页表和用户内存复制：引导初始化。
@param kind 本次操作使用的 种类 值。
@param device_tree 固件传入的设备树物理地址。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
@details 流程由 BootInitializeFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-ca094a83ada9f57d"></a>

## `xv::memory::BootInitializeFlow::discover`

```doxygen
@fn static void xv::memory::BootInitializeFlow::discover(Context &c) noexcept
@brief 从固件提供的 DTB 发现候选 RAM 描述。
@details 属于 memory/boot，未扩大排除范围；候选内存描述与源保持原函数局部所有权，保留发现、DTB 居住范围、初始化失败诊断和原报告顺序。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-02f29380e4420738"></a>

## `xv::memory::BootInitializeFlow::initialize`

```doxygen
@fn static void xv::memory::BootInitializeFlow::initialize(Context &c) noexcept
@brief 使用原内核末尾符号初始化选定分配器。
@details 属于 memory/boot，未扩大排除范围；候选内存描述与源保持原函数局部所有权，保留发现、DTB 居住范围、初始化失败诊断和原报告顺序。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-92da10524e4a727d"></a>

## `xv::memory::BootInitializeFlow::invalid_layout`

```doxygen
@fn static void xv::memory::BootInitializeFlow::invalid_layout(Context &) noexcept
@brief 按原无效布局或元数据不足诊断终止。
@details 属于 memory/boot，未扩大排除范围；候选内存描述与源保持原函数局部所有权，保留发现、DTB 居住范围、初始化失败诊断和原报告顺序。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-eae3c99cdef2d9fd"></a>

## `xv::memory::BootInitializeFlow::invalid_tree`

```doxygen
@fn static void xv::memory::BootInitializeFlow::invalid_tree(Context &) noexcept
@brief 按原无效设备树诊断终止。
@details 属于 memory/boot，未扩大排除范围；候选内存描述与源保持原函数局部所有权，保留发现、DTB 居住范围、初始化失败诊断和原报告顺序。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-b04607679bc30c19"></a>

## `xv::memory::BootInitializeFlow::ok`

```doxygen
@fn static bool xv::memory::BootInitializeFlow::ok(Context &c) noexcept
@brief 检查发现或分配器初始化是否成功。
@details 属于 memory/boot，未扩大排除范围；候选内存描述与源保持原函数局部所有权，保留发现、DTB 居住范围、初始化失败诊断和原报告顺序。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-1f3f05c5ba65af6c"></a>

## `xv::memory::BootInitializeFlow::outside`

```doxygen
@fn static void xv::memory::BootInitializeFlow::outside(Context &) noexcept
@brief 按原 DTB 位于 RAM 外诊断终止。
@details 属于 memory/boot，未扩大排除范围；候选内存描述与源保持原函数局部所有权，保留发现、DTB 居住范围、初始化失败诊断和原报告顺序。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-e94df4064c4365f5"></a>

## `xv::memory::BootInitializeFlow::report`

```doxygen
@fn static void xv::memory::BootInitializeFlow::report(Context &c) noexcept
@brief 输出当前 RAM、元数据与可用页统计。
@details 属于 memory/boot，未扩大排除范围；候选内存描述与源保持原函数局部所有权，保留发现、DTB 居住范围、初始化失败诊断和原报告顺序。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-c14473c4e8d3b353"></a>

## `xv::memory::BootInitializeFlow::resident`

```doxygen
@fn static bool xv::memory::BootInitializeFlow::resident(Context &c) noexcept
@brief 检查固件 DTB 起点位于发现的 RAM 内。
@details 属于 memory/boot，未扩大排除范围；候选内存描述与源保持原函数局部所有权，保留发现、DTB 居住范围、初始化失败诊断和原报告顺序。 下一节点由图策略声明，业务结果与控制信号分离。
```
