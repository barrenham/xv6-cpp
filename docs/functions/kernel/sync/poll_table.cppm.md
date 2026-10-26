# `kernel/sync/poll_table.cppm`

开发职责：锁所有权、屏蔽中断与对象等待登记。

源码：[打开实际文件](../../../../kernel/sync/poll_table.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.sync:poll`。

## 函数导航

- [`xv::sync::PollTable::PollTable`](poll_table.cppm.md#fn-6c40c21483e56aaa)
- [`xv::sync::PollTable::events`](poll_table.cppm.md#fn-e72b69d8281ca8d1)
- [`xv::sync::PollTable::request`](poll_table.cppm.md#fn-3594871871bd36bf)
- [`xv::sync::PollTable::watch`](poll_table.cppm.md#fn-0d0421e365c2829b)
- [`xv::sync::PollTable::~PollTable`](poll_table.cppm.md#fn-47ddae80b10d0203)

<a id="fn-0d0421e365c2829b"></a>

## `xv::sync::PollTable::watch`

```doxygen
@fn virtual void xv::sync::PollTable::watch(void *channel)
@brief 锁所有权、屏蔽中断与对象等待登记：登记一个影响就绪条件的对象通道。
@param channel 等待条件或通知对象的地址键；不能在未结束登记时销毁对应对象。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-e72b69d8281ca8d1"></a>

## `xv::sync::PollTable::events`

```doxygen
@fn unsigned xv::sync::PollTable::events() const
@brief 锁所有权、屏蔽中断与对象等待登记：读取或计算 `events_`，返回当前查询结果。
@return 各分支实际返回：`events_`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-3594871871bd36bf"></a>

## `xv::sync::PollTable::request`

```doxygen
@fn void xv::sync::PollTable::request(unsigned events)
@brief 锁所有权、屏蔽中断与对象等待登记：记录本次 poll 查询关注的事件位。
@param events 本次操作使用的 事件位 值。
@details 本函数可见的状态更新包括：`events_ = events;`。局部临时变量与对象字段应按作用域区分。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-6c40c21483e56aaa"></a>

## `xv::sync::PollTable::PollTable`

```doxygen
@fn constexpr xv::sync::PollTable::PollTable()
@brief 构造锁所有权、屏蔽中断与对象等待登记，建立本重载要求的成员初值和借用关系。
@details 使用编译器生成的默认成员行为。普通标量是否被初始化取决于成员默认初始化器和调用方的值初始化方式，不能把 default 一概当作清零。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-47ddae80b10d0203"></a>

## `xv::sync::PollTable::~PollTable`

```doxygen
@fn xv::sync::PollTable::~PollTable()
@brief 结束锁所有权、屏蔽中断与对象等待登记的对象生命周期，并执行其显式清理或成员析构。
@details 析构函数自身没有额外语句，成员对象仍按 C++ 析构顺序处理。对象存储的归还由拥有者或外层释放入口负责。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```
