# `kernel/trap/dispatch/trap_dispatcher.cpp`

开发职责：陷阱快照、事件分派、设备服务及特权级返回。

源码：[打开实际文件](../../../../../kernel/trap/dispatch/trap_dispatcher.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.trap`。

直接模块依赖：`:platform`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::trap::TrapDispatcher::dispatch`](trap_dispatcher.cpp.md#fn-d2dcba4fb1b087d6)

<a id="fn-d2dcba4fb1b087d6"></a>

## `xv::trap::TrapDispatcher::dispatch`

```doxygen
@fn TrapAction xv::trap::TrapDispatcher::dispatch(const TrapEvent &event)
@brief 陷阱处理器分派表：按输入事件或系统调用编号选择处理路径。
@param event 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@return 各分支实际返回：`handlers_[i]->handle(event)`；`TrapAction::Unhandled`。应结合上面的条件检查区分正常值、空值和错误值。
@details 编排由 DispatchFlow 的模板图声明；具名阶段负责业务操作，控制策略负责下一节点。原有等待协议、引用所有权和错误返回约定保留。
```
