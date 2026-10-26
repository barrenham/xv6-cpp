# `kernel/boot/main.cpp`

开发职责：启动、每 hart 初始化和进入内核的交接。

源码：[打开实际文件](../../../../kernel/boot/main.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.boot`。

直接模块依赖：`:state`, `xv.print`, `xv.trap`, `xv.fs`, `xv.process`, `xv.drivers`, `xv.memory`, `xv.net`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::boot::run`](main.cpp.md#fn-83525a5281519558)
- [`xv::boot::initialize_bootstrap_hart`](main.cpp.md#fn-2ce54ce3db33e308)
- [`xv::boot::initialize_secondary_hart`](main.cpp.md#fn-c64bfe6e40c53cad)

<a id="fn-83525a5281519558"></a>

## `xv::boot::run`

```doxygen
@fn void xv::boot::run()
@brief 启动、每 hart 初始化和进入内核的交接：运行。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::cpu_id` → `initialize_bootstrap_hart` → `__atomic_store_n` → `initialize_secondary_hart` → `xv::process::scheduler` → `run`。
@details 需要特别跟踪的条件：`xv::process::cpu_id() == 0`。这些条件决定检查、重试或提前返回路径。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-c64bfe6e40c53cad"></a>

## `xv::boot::initialize_secondary_hart`

```doxygen
@fn void xv::boot::initialize_secondary_hart()
@brief 启动、每 hart 初始化和进入内核的交接：初始化次级 harthart。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`__atomic_load_n` → `volatile` → `xv::print::printk` → `xv::process::cpu_id` → `xv::memory::kernel_page_table` → `activate_hart` → `xv::trap::initialize_hart` → `xv::drivers::initialize_hart`。
@details 需要特别跟踪的条件：`__atomic_load_n(&started, __ATOMIC_ACQUIRE) == 0`。这些条件决定检查、重试或提前返回路径。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```
<a id="fn-2ce54ce3db33e308"></a>

## `xv::boot::initialize_bootstrap_hart`

```doxygen
@fn void xv::boot::initialize_bootstrap_hart()
@brief 按依赖顺序初始化启动核各服务；网络启动后逐接口输出实际名称、MTU、管理状态及 IPv4 地址列表。
```
