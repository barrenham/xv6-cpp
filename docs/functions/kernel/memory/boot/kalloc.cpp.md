# `kernel/memory/boot/kalloc.cpp`

开发职责：物理页所有权、地址范围、页表和用户内存复制。

源码：[打开实际文件](../../../../../kernel/memory/boot/kalloc.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.memory`。

直接模块依赖：`xv.print`, `xv.runtime`, `:detail`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

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
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`source` → `device_tree.pointer` → `source.discover` → `detail::fail` → `memory.ram` → `contains` → `initialize` → `PhysicalAddress::from_pointer` → `allocator` → `layout` → `xv::print::printk` → `layout.ram`。
@details 需要特别跟踪的条件：`!source.discover(memory)`；`!memory.ram().contains(device_tree)`；`!initialize(kind, memory, PhysicalAddress::from_pointer(end))`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```
