# `kernel/boot/start.cpp`

开发职责：启动、每 hart 初始化和进入内核的交接。

源码：[打开实际文件](../../../../kernel/boot/start.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.boot`。

直接模块依赖：`xv.arch`, `:state`, `xv.trap`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::boot::detail::device_tree`](start.cpp.md#fn-6e4debad43d43f5f)
- [`xv::boot::enter`](start.cpp.md#fn-caadfb68f7c23853)
- [`xv::boot::initialize_timer`](start.cpp.md#fn-b0eb3ac8b28cbcdd)

<a id="fn-caadfb68f7c23853"></a>

## `xv::boot::enter`

```doxygen
@fn void xv::boot::enter(unsigned long hart_id, unsigned long device_tree)
@brief 启动、每 hart 初始化和进入内核的交接：进入。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
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
