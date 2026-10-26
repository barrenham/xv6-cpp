# `kernel/trap/arch/riscv/entry.cpp`

开发职责：陷阱快照、事件分派、设备服务及特权级返回。

源码：[打开实际文件](../../../../../../kernel/trap/arch/riscv/entry.cpp)。文档不会加入源码或生成额外 C++ 声明。

直接模块依赖：`xv.trap`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`kerneltrap`](entry.cpp.md#fn-3307905e8431d793)
- [`usertrap`](entry.cpp.md#fn-e31b5236fa8fe95c)

<a id="fn-e31b5236fa8fe95c"></a>

## `usertrap`

```doxygen
@fn unsigned long usertrap()
@brief 陷阱快照、事件分派、设备服务及特权级返回：读取或计算 `xv::trap::controller().handle_user()`，返回当前查询结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::trap::controller` → `handle_user`。
@return 各分支实际返回：`xv::trap::controller().handle_user()`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-3307905e8431d793"></a>

## `kerneltrap`

```doxygen
@fn void kerneltrap()
@brief 陷阱快照、事件分派、设备服务及特权级返回：kerneltrap。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::trap::controller` → `handle_kernel`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```
