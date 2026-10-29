# `kernel/process/groups/process_groups.cppm`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../kernel/process/groups/process_groups.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process:groups`。

直接模块依赖：`:identity`, `xv.sync`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::process::GroupVisitor::GroupVisitor`](process_groups.cppm.md#fn-470d6373cc49b3b8)
- [`xv::process::GroupVisitor::visit`](process_groups.cppm.md#fn-cba1cf155578f2f4)
- [`xv::process::GroupVisitor::~GroupVisitor`](process_groups.cppm.md#fn-2cf3e834a0d6858c)
- [`xv::process::ProcessGroups::GroupKey::equal`](process_groups.cppm.md#fn-dfddda6e8f24b5f9)
- [`xv::process::ProcessGroups::GroupKey::hash`](process_groups.cppm.md#fn-c9894adb2ba0f862)
- [`xv::process::ProcessGroups::GroupKey::key`](process_groups.cppm.md#fn-4dcc877d1e918cb2)
- [`xv::process::ProcessGroups::ProcessGroups`](process_groups.cppm.md#fn-1458c6b76a1ce516)
- [`xv::process::ProcessGroups::allocate_group`](process_groups.cpp.md#fn-1dbdc5852a1b5676)
- [`xv::process::ProcessGroups::attach`](process_groups.cpp.md#fn-99e88e769ce178e2)
- [`xv::process::ProcessGroups::claim_terminal`](terminal_control.cpp.md#fn-8a56e2b14cf9b154)
- [`xv::process::ProcessGroups::create_initial`](process_groups.cpp.md#fn-0b49bfae1c638cc0)
- [`xv::process::ProcessGroups::create_session`](process_groups.cpp.md#fn-aa7ea8358c3e4e35)
- [`xv::process::ProcessGroups::detach`](process_groups.cpp.md#fn-73bbed264e0c09a5)
- [`xv::process::ProcessGroups::detach_terminal`](terminal_control.cpp.md#fn-f799f3318f792fa1)
- [`xv::process::ProcessGroups::did_exec`](process_groups.cpp.md#fn-38e8e67dcb920d6f)
- [`xv::process::ProcessGroups::exit_terminal`](terminal_control.cpp.md#fn-a4ca6b6dd1789097)
- [`xv::process::ProcessGroups::find_group`](process_groups.cpp.md#fn-2e0fd0545f91a391)
- [`xv::process::ProcessGroups::find_member`](process_groups.cpp.md#fn-17454b9eb064996b)
- [`xv::process::ProcessGroups::foreground`](terminal_control.cpp.md#fn-da38cc2bc5cbb6be)
- [`xv::process::ProcessGroups::get_group`](process_groups.cpp.md#fn-ad044f30e0ca5762)
- [`xv::process::ProcessGroups::get_session`](process_groups.cpp.md#fn-8a811a35de5d5380)
- [`xv::process::ProcessGroups::inherit`](process_groups.cpp.md#fn-fb259ac360aabda0)
- [`xv::process::ProcessGroups::initialize`](process_groups.cpp.md#fn-64a3ff755b49e0b6)
- [`xv::process::ProcessGroups::may_read_terminal`](terminal_control.cpp.md#fn-7e21e578da02ef0b)
- [`xv::process::ProcessGroups::release`](process_groups.cpp.md#fn-59b7282a49e6dc1a)
- [`xv::process::ProcessGroups::reparent`](process_groups.cpp.md#fn-02fe7151a3441d92)
- [`xv::process::ProcessGroups::set_foreground`](terminal_control.cpp.md#fn-aeb712077b7a077b)
- [`xv::process::ProcessGroups::set_group`](process_groups.cpp.md#fn-e0e5fac1455dc712)
- [`xv::process::ProcessGroups::visit_foreground`](terminal_control.cpp.md#fn-f94a9086969893f9)
- [`xv::process::ProcessGroups::visit_group`](process_groups.cpp.md#fn-13b8d39cbdbc6f32)

<a id="fn-cba1cf155578f2f4"></a>

## `xv::process::GroupVisitor::visit`

```doxygen
@fn virtual void xv::process::GroupVisitor::visit(ProcessMembership &member)
@brief 进程对象生命周期、组身份、调度、等待与信号：处理稳定遍历当前访问到的对象。
@param member 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-470d6373cc49b3b8"></a>

## `xv::process::GroupVisitor::GroupVisitor`

```doxygen
@fn constexpr xv::process::GroupVisitor::GroupVisitor()
@brief 构造进程对象生命周期、组身份、调度、等待与信号，建立本重载要求的成员初值和借用关系。
@details 使用编译器生成的默认成员行为。普通标量是否被初始化取决于成员默认初始化器和调用方的值初始化方式，不能把 default 一概当作清零。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-2cf3e834a0d6858c"></a>

## `xv::process::GroupVisitor::~GroupVisitor`

```doxygen
@fn xv::process::GroupVisitor::~GroupVisitor()
@brief 结束进程对象生命周期、组身份、调度、等待与信号的对象生命周期，并执行其显式清理或成员析构。
@details 析构函数自身没有额外语句，成员对象仍按 C++ 析构顺序处理。对象存储的归还由拥有者或外层释放入口负责。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-1458c6b76a1ce516"></a>

## `xv::process::ProcessGroups::ProcessGroups`

```doxygen
@fn constexpr xv::process::ProcessGroups::ProcessGroups()
@brief 构造会话和进程组登记，建立本重载要求的成员初值和借用关系。
@details 使用编译器生成的默认成员行为。普通标量是否被初始化取决于成员默认初始化器和调用方的值初始化方式，不能把 default 一概当作清零。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-4dcc877d1e918cb2"></a>

## `xv::process::ProcessGroups::GroupKey::key`

```doxygen
@fn static int xv::process::ProcessGroups::GroupKey::key(const ProcessGroup &group)
@brief 取得进程组的稳定组 ID。
@details 遵守原进程锁、就绪队列锁、Runnable 生命周期和上下文交接约定；详见可选时钟调度设计。
```

<a id="fn-c9894adb2ba0f862"></a>

## `xv::process::ProcessGroups::GroupKey::hash`

```doxygen
@fn static unsigned long xv::process::ProcessGroups::GroupKey::hash(int id)
@brief 混合组 ID 以选择哈希桶。
@details 遵守原进程锁、就绪队列锁、Runnable 生命周期和上下文交接约定；详见可选时钟调度设计。
```

<a id="fn-dfddda6e8f24b5f9"></a>

## `xv::process::ProcessGroups::GroupKey::equal`

```doxygen
@fn static bool xv::process::ProcessGroups::GroupKey::equal(int a, int b)
@brief 比较组 ID 是否相同。
@details 遵守原进程锁、就绪队列锁、Runnable 生命周期和上下文交接约定；详见可选时钟调度设计。
```

组 ID 哈希索引的所有权与复杂度见 [基础容器](../../../../kernel-containers.md)。
