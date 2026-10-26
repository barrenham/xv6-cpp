# `kernel/boot/start.cpp`

开发职责：启动、每 hart 初始化和进入内核的交接。

源码：[打开实际文件](../../../../kernel/boot/start.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.boot`。

直接模块依赖：`xv.arch`, `:state`, `xv.trap`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::boot::enter`](start.cpp.md#fn-caadfb68f7c23853)
- [`xv::boot::detail::device_tree`](start.cpp.md#fn-6e4debad43d43f5f)
- [`xv::boot::initialize_timer`](start.cpp.md#fn-b0eb3ac8b28cbcdd)

<a id="fn-caadfb68f7c23853"></a>

## `xv::boot::enter`

```doxygen
@fn void xv::boot::enter(unsigned long hart_id, unsigned long device_tree)
@brief 启动、每 hart 初始化和进入内核的交接：进入。
@param hart_id 当前启动 hart 的编号。
@param device_tree 固件传入的设备树物理地址。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::arch::r_mstatus` → `xv::arch::w_mstatus` → `xv::arch::w_mepc` → `xv::arch::w_satp` → `xv::arch::w_medeleg` → `xv::arch::w_mideleg` → `xv::arch::w_sie` → `xv::arch::r_sie` → `xv::arch::w_pmpaddr0` → `xv::arch::w_pmpcfg0` → `xv::arch::w_menvcfg` → `xv::arch::r_menvcfg`。
@details 需要特别跟踪的条件：`hart_id == 0`。这些条件决定检查、重试或提前返回路径。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-b0eb3ac8b28cbcdd"></a>

## `xv::boot::initialize_timer`

```doxygen
@fn void xv::boot::initialize_timer()
@brief 启动、每 hart 初始化和进入内核的交接：初始化定时器。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::arch::w_menvcfg` → `xv::arch::r_menvcfg` → `xv::arch::w_mcounteren` → `xv::arch::r_mcounteren` → `xv::arch::w_stimecmp` → `xv::arch::r_time`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-6e4debad43d43f5f"></a>

## `xv::boot::detail::device_tree`

```doxygen
@fn unsigned long xv::boot::detail::device_tree()
@brief 启动、每 hart 初始化和进入内核的交接：读取或计算 `firmware_device_tree`，返回当前查询结果。
@return 各分支实际返回：`firmware_device_tree`。应结合上面的条件检查区分正常值、空值和错误值。
```
