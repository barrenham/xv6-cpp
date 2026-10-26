# `kernel/drivers/interrupt/interrupt_router.cppm`

开发职责：设备对象、MMIO、设备中断与终端数据流。

源码：[打开实际文件](../../../../../kernel/drivers/interrupt/interrupt_router.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.drivers:router`。

直接模块依赖：`:interrupt`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::drivers::InterruptRouter::InterruptRouter`](interrupt_router.cppm.md#fn-3086635f72a22b8b)
- [`xv::drivers::InterruptRouter::InterruptRouter`](interrupt_router.cppm.md#fn-1752422b105d7b3f)
- [`xv::drivers::InterruptRouter::dispatch`](interrupt_router.cpp.md#fn-d2d94b71a3549485)
- [`xv::drivers::InterruptRouter::initialize`](interrupt_router.cpp.md#fn-8e59e19503e86de5)
- [`xv::drivers::InterruptRouter::initialize_hart`](interrupt_router.cpp.md#fn-4efe08b4f15644d0)
- [`xv::drivers::InterruptRouter::operator=`](interrupt_router.cppm.md#fn-0531dc9cf7b81b4c)

<a id="fn-3086635f72a22b8b"></a>

## `xv::drivers::InterruptRouter::InterruptRouter`

```doxygen
@fn constexpr xv::drivers::InterruptRouter::InterruptRouter(InterruptController &controller, InterruptDevice *const *devices, unsigned count)
@brief 构造外部中断路由器，建立本重载要求的成员初值和借用关系。
@param controller 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param devices 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param count 本次操作的数量上限。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`controller_` → `devices_` → `count_`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-1752422b105d7b3f"></a>

## `xv::drivers::InterruptRouter::InterruptRouter`

```doxygen
@fn xv::drivers::InterruptRouter::InterruptRouter(const InterruptRouter &)
@brief 外部中断路由器的禁用操作：在编译期拒绝此种复制或赋值。
@details 该成员显式声明为 deleted，不存在可在运行期调用的实现。它阻止绕开对象的引用/锁/所有权规则创建第二个拥有者。
@return 无运行期返回路径；使用该操作会导致编译失败。
```

<a id="fn-0531dc9cf7b81b4c"></a>

## `xv::drivers::InterruptRouter::operator=`

```doxygen
@fn InterruptRouter &xv::drivers::InterruptRouter::operator=(const InterruptRouter &)
@brief 外部中断路由器的禁用操作：在编译期拒绝此种复制或赋值。
@details 该成员显式声明为 deleted，不存在可在运行期调用的实现。它阻止绕开对象的引用/锁/所有权规则创建第二个拥有者。
@return 无运行期返回路径；使用该操作会导致编译失败。
```
