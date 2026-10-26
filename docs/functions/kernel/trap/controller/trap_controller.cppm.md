# `kernel/trap/controller/trap_controller.cppm`

开发职责：陷阱快照、事件分派、设备服务及特权级返回。

源码：[打开实际文件](../../../../../kernel/trap/controller/trap_controller.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.trap:controller`。

直接模块依赖：`:dispatch`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::trap::controller`](../service.cpp.md#fn-600af30c3d4e40d2)
- [`xv::trap::TrapController::TrapController`](trap_controller.cppm.md#fn-99f5597352abd02c)
- [`xv::trap::TrapController::TrapController`](trap_controller.cppm.md#fn-2d136298ad6ddafd)
- [`xv::trap::TrapController::handle_kernel`](trap_controller.cpp.md#fn-1e01ca83cbbc98f6)
- [`xv::trap::TrapController::handle_user`](trap_controller.cpp.md#fn-91cdce7331da1c4b)
- [`xv::trap::TrapController::initialize_hart`](trap_controller.cpp.md#fn-afc1472bab28e034)
- [`xv::trap::TrapController::operator=`](trap_controller.cppm.md#fn-031529e9dcb30872)
- [`xv::trap::TrapController::prepare_return`](trap_controller.cpp.md#fn-0713205e94c82bbd)
- [`xv::trap::TrapController::return_to_user`](trap_controller.cpp.md#fn-d181f139ca490928)

<a id="fn-99f5597352abd02c"></a>

## `xv::trap::TrapController::TrapController`

```doxygen
@fn constexpr explicit xv::trap::TrapController::TrapController(TrapDispatcher &dispatcher)
@brief 构造陷阱控制器，建立本重载要求的成员初值和借用关系。
@param dispatcher 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`dispatcher_`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-2d136298ad6ddafd"></a>

## `xv::trap::TrapController::TrapController`

```doxygen
@fn xv::trap::TrapController::TrapController(const TrapController &)
@brief 陷阱控制器的禁用操作：在编译期拒绝此种复制或赋值。
@details 该成员显式声明为 deleted，不存在可在运行期调用的实现。它阻止绕开对象的引用/锁/所有权规则创建第二个拥有者。
@return 无运行期返回路径；使用该操作会导致编译失败。
```

<a id="fn-031529e9dcb30872"></a>

## `xv::trap::TrapController::operator=`

```doxygen
@fn TrapController &xv::trap::TrapController::operator=(const TrapController &)
@brief 陷阱控制器的禁用操作：在编译期拒绝此种复制或赋值。
@details 该成员显式声明为 deleted，不存在可在运行期调用的实现。它阻止绕开对象的引用/锁/所有权规则创建第二个拥有者。
@return 无运行期返回路径；使用该操作会导致编译失败。
```
