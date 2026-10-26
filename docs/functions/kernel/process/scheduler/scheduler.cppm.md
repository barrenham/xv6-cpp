# `kernel/process/scheduler/scheduler.cppm`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../kernel/process/scheduler/scheduler.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process:scheduler`。

直接模块依赖：`:process`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::process::scheduler`](../lifecycle/service.cpp.md#fn-7abeaa33d65489ea)
- [`xv::process::Scheduler::Scheduler`](scheduler.cppm.md#fn-087112f2f52c9a12)
- [`xv::process::Scheduler::Scheduler`](scheduler.cppm.md#fn-7849fdcb99edf01f)
- [`xv::process::Scheduler::make_runnable`](scheduler.cppm.md#fn-5d21738df9c371cc)
- [`xv::process::Scheduler::name`](scheduler.cppm.md#fn-9a68b702ad5864cb)
- [`xv::process::Scheduler::operator=`](scheduler.cppm.md#fn-4cdde651a6f26e2c)
- [`xv::process::Scheduler::run`](scheduler.cppm.md#fn-38b4360e80422d03)
- [`xv::process::Scheduler::suspend`](scheduler.cppm.md#fn-34e8f43299600c34)
- [`xv::process::Scheduler::yield`](scheduler.cppm.md#fn-511de5124105eee7)
- [`xv::process::Scheduler::~Scheduler`](scheduler.cppm.md#fn-0ee984bb6da8f0ae)

<a id="fn-7849fdcb99edf01f"></a>

## `xv::process::Scheduler::Scheduler`

```doxygen
@fn xv::process::Scheduler::Scheduler(const Scheduler &)
@brief 进程对象生命周期、组身份、调度、等待与信号的禁用操作：在编译期拒绝此种复制或赋值。
@details 该成员显式声明为 deleted，不存在可在运行期调用的实现。它阻止绕开对象的引用/锁/所有权规则创建第二个拥有者。
@return 无运行期返回路径；使用该操作会导致编译失败。
```

<a id="fn-4cdde651a6f26e2c"></a>

## `xv::process::Scheduler::operator=`

```doxygen
@fn Scheduler &xv::process::Scheduler::operator=(const Scheduler &)
@brief 进程对象生命周期、组身份、调度、等待与信号的禁用操作：在编译期拒绝此种复制或赋值。
@details 该成员显式声明为 deleted，不存在可在运行期调用的实现。它阻止绕开对象的引用/锁/所有权规则创建第二个拥有者。
@return 无运行期返回路径；使用该操作会导致编译失败。
```

<a id="fn-9a68b702ad5864cb"></a>

## `xv::process::Scheduler::name`

```doxygen
@fn virtual const char *xv::process::Scheduler::name() const
@brief 进程对象生命周期、组身份、调度、等待与信号：名称。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 返回类型为 `const char *() const`；本条目没有可独立分析的普通函数体，具体返回约定见对应实现及调用方。
```

<a id="fn-38b4360e80422d03"></a>

## `xv::process::Scheduler::run`

```doxygen
@fn [[noreturn]] virtual void xv::process::Scheduler::run()
@brief 进程对象生命周期、组身份、调度、等待与信号：运行。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 不返回正常调用点；后续执行流由异常终止、调度或特权级切换接管。
```

<a id="fn-34e8f43299600c34"></a>

## `xv::process::Scheduler::suspend`

```doxygen
@fn virtual void xv::process::Scheduler::suspend()
@brief 进程对象生命周期、组身份、调度、等待与信号：在规定的锁状态下切回调度器。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-511de5124105eee7"></a>

## `xv::process::Scheduler::yield`

```doxygen
@fn virtual void xv::process::Scheduler::yield()
@brief 进程对象生命周期、组身份、调度、等待与信号：把当前进程重新排队并主动让出 CPU。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-5d21738df9c371cc"></a>

## `xv::process::Scheduler::make_runnable`

```doxygen
@fn virtual void xv::process::Scheduler::make_runnable(Process &process)
@brief 进程对象生命周期、组身份、调度、等待与信号：把受锁保护的进程发布到就绪队列。
@param process 被操作的稳定进程对象，存活期必须覆盖本次访问。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-087112f2f52c9a12"></a>

## `xv::process::Scheduler::Scheduler`

```doxygen
@fn constexpr xv::process::Scheduler::Scheduler()
@brief 构造进程对象生命周期、组身份、调度、等待与信号，建立本重载要求的成员初值和借用关系。
@details 使用编译器生成的默认成员行为。普通标量是否被初始化取决于成员默认初始化器和调用方的值初始化方式，不能把 default 一概当作清零。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-0ee984bb6da8f0ae"></a>

## `xv::process::Scheduler::~Scheduler`

```doxygen
@fn xv::process::Scheduler::~Scheduler()
@brief 结束进程对象生命周期、组身份、调度、等待与信号的对象生命周期，并执行其显式清理或成员析构。
@details 析构函数自身没有额外语句，成员对象仍按 C++ 析构顺序处理。对象存储的归还由拥有者或外层释放入口负责。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```
