# `kernel/boot/main.cpp`

开发职责：启动、每 hart 初始化和进入内核的交接。

源码：[打开实际文件](../../../../kernel/boot/main.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.boot`。

直接模块依赖：`:state`, `xv.print`, `xv.trap`, `xv.fs`, `xv.process`, `xv.drivers`, `xv.memory`, `xv.net`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`enter_scheduler_stack`](main.cpp.md#fn-33edc3fb91e368c4)
- [`scheduler_on_stack`](main.cpp.md#fn-6869a6cc40430e52)
- [`xv::boot::initialize_bootstrap_hart`](main.cpp.md#fn-2ce54ce3db33e308)
- [`xv::boot::initialize_secondary_hart`](main.cpp.md#fn-c64bfe6e40c53cad)
- [`xv::boot::run`](main.cpp.md#fn-83525a5281519558)

<a id="fn-83525a5281519558"></a>

## `xv::boot::run`

```doxygen
@fn void xv::boot::run()
@brief 完成各 hart 初始化后切换到独立调度栈。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
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

<a id="fn-33edc3fb91e368c4"></a>

## `enter_scheduler_stack`

```doxygen
@fn void enter_scheduler_stack(unsigned long top)
@brief 将当前 hart 的 SP 切换到调度栈顶。
@details 汇编 mv sp,a0 后尾调用 scheduler_on_stack，旧启动调用帧不再返回；传入地址必须为已映射初始页的预留区域顶端且保持 ABI 对齐。
```

<a id="fn-6869a6cc40430e52"></a>

## `scheduler_on_stack`

```doxygen
@fn void scheduler_on_stack()
@brief 在每 hart 的新调度栈进入调度循环。
@details 由汇编切换 SP 后直接进入，全程不返回；调度循环仍管理 cpu.current、进程锁与上下文切换。
```
