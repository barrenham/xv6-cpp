# `kernel/process/arch/cpu.cpp`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../kernel/process/arch/cpu.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process`。

直接模块依赖：`:cpu`, `xv.arch`, `xv.sync`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::process::cpu_at`](cpu.cpp.md#fn-ad114cb3f25a8114)
- [`xv::process::cpu_id`](cpu.cpp.md#fn-38c0b9f44adde749)
- [`xv::process::current`](cpu.cpp.md#fn-3674a9cba9c27758)
- [`xv::process::current_cpu`](cpu.cpp.md#fn-eaa0d9fcd809861d)

<a id="fn-38c0b9f44adde749"></a>

## `xv::process::cpu_id`

```doxygen
@fn int xv::process::cpu_id()
@brief 进程对象生命周期、组身份、调度、等待与信号：读取当前 hart 的编号。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::arch::r_tp`。
@return 各分支实际返回：`id`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-eaa0d9fcd809861d"></a>

## `xv::process::current_cpu`

```doxygen
@fn CpuState *xv::process::current_cpu(void)
@brief 进程对象生命周期、组身份、调度、等待与信号：取得当前 hart 的 CPU 状态对象。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`cpu_id`。
@return 各分支实际返回：`c`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-3674a9cba9c27758"></a>

## `xv::process::current`

```doxygen
@fn Process *xv::process::current(void)
@brief 进程对象生命周期、组身份、调度、等待与信号：取得当前 hart 正在执行的进程。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::sync::push_interrupts_off` → `current_cpu` → `xv::sync::pop_interrupts_off`。
@return 各分支实际返回：`p`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-ad114cb3f25a8114"></a>

## `xv::process::cpu_at`

```doxygen
@fn CpuState *xv::process::cpu_at(int index)
@brief 进程对象生命周期、组身份、调度、等待与信号：读取或计算 `&cpus[index]`，返回当前查询结果。
@param index 数组、CPU 或页面的索引。
@return 各分支实际返回：`&cpus[index]`。应结合上面的条件检查区分正常值、空值和错误值。
```
