# `kernel/trap/service.cpp`

开发职责：陷阱快照、事件分派、设备服务及特权级返回。

源码：[打开实际文件](../../../../kernel/trap/service.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.trap`。

直接模块依赖：`:handlers`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::trap::clock`](service.cpp.md#fn-7c77589a1cbc708a)
- [`xv::trap::controller`](service.cpp.md#fn-600af30c3d4e40d2)
- [`xv::trap::initialize`](service.cpp.md#fn-d8b22eb1636b2b5c)
- [`xv::trap::initialize_hart`](service.cpp.md#fn-a54f903e3675d4e8)
- [`xv::trap::prepare_return`](service.cpp.md#fn-0968469d414ee0fc)
- [`xv::trap::return_to_user`](service.cpp.md#fn-3733f8d54bf76c4c)

<a id="fn-7c77589a1cbc708a"></a>

## `xv::trap::clock`

```doxygen
@fn TickClock &xv::trap::clock()
@brief 陷阱快照、事件分派、设备服务及特权级返回：读取或计算 `system_clock`，返回当前查询结果。
@return 各分支实际返回：`system_clock`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-600af30c3d4e40d2"></a>

## `xv::trap::controller`

```doxygen
@fn TrapController &xv::trap::controller()
@brief 陷阱快照、事件分派、设备服务及特权级返回：读取或计算 `system_controller`，返回当前查询结果。
@return 各分支实际返回：`system_controller`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-d8b22eb1636b2b5c"></a>

## `xv::trap::initialize`

```doxygen
@fn void xv::trap::initialize()
@brief 陷阱快照、事件分派、设备服务及特权级返回：建立该对象开始工作所需的初始状态。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`system_clock.initialize`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-a54f903e3675d4e8"></a>

## `xv::trap::initialize_hart`

```doxygen
@fn void xv::trap::initialize_hart()
@brief 陷阱快照、事件分派、设备服务及特权级返回：建立当前 hart 的局部运行状态。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`system_controller.initialize_hart`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-0968469d414ee0fc"></a>

## `xv::trap::prepare_return`

```doxygen
@fn void xv::trap::prepare_return()
@brief 陷阱快照、事件分派、设备服务及特权级返回：完成返回用户态前的信号检查与架构准备。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`system_controller.prepare_return`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-3733f8d54bf76c4c"></a>

## `xv::trap::return_to_user`

```doxygen
@fn void xv::trap::return_to_user()
@brief 陷阱快照、事件分派、设备服务及特权级返回：准备并进入不返回的用户态切换路径。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`system_controller.return_to_user`。
@return 各分支实际返回：`_to_user() { system_controller.return_to_user()`。应结合上面的条件检查区分正常值、空值和错误值。
```
