# `kernel/sync/sleep/sleeplock.cpp`

开发职责：锁所有权、屏蔽中断与对象等待登记。

源码：[打开实际文件](../../../../../kernel/sync/sleep/sleeplock.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.sync`。

直接模块依赖：`:platform`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::sync::SleepLock::acquire`](sleeplock.cpp.md#fn-0df6c25631ea54c1)
- [`xv::sync::SleepLock::acquire_interruptible`](sleeplock.cpp.md#fn-4bf66df4f890a6a5)
- [`xv::sync::SleepLock::available`](sleeplock.cpp.md#fn-cb0bae526473552b)
- [`xv::sync::SleepLock::holding`](sleeplock.cpp.md#fn-d8314f38391b20f3)
- [`xv::sync::SleepLock::init`](sleeplock.cpp.md#fn-7372bd87bf9732cf)
- [`xv::sync::SleepLock::release`](sleeplock.cpp.md#fn-9c53ea1cb256794a)
- [`xv::sync::SleepLock::try_acquire`](sleeplock.cpp.md#fn-e399786b3d75952b)

<a id="fn-7372bd87bf9732cf"></a>

## `xv::sync::SleepLock::init`

```doxygen
@fn void xv::sync::SleepLock::init(const char *label)
@brief 睡眠锁：init。
@param label 保存在对象中的调试标签，字符串需在使用期间有效。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`state_lock_.init`。
@details 本函数可见的状态更新包括：`name_ = label;`；`locked_ = false;`；`pid_ = 0;`。局部临时变量与对象字段应按作用域区分。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-0df6c25631ea54c1"></a>

## `xv::sync::SleepLock::acquire`

```doxygen
@fn void xv::sync::SleepLock::acquire()
@brief 睡眠锁：取得锁所有权。
@details 持 state_lock_ 检查 locked_；忙时先登记 this 通道，再释放状态锁并睡眠，恢复后重新获取状态锁重查。最后登记当前 PID 为拥有者；业务临界区不持有 state_lock_。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`state_lock_.acquire` → `detail::current_pid` → `detail::fail` → `detail::prepare_sleep` → `state_lock_.release` → `detail::sleep_process`。
@details 本函数可见的状态更新包括：`locked_ = true;`；`pid_ = detail::current_pid();`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`locked_`；`pid_ == detail::current_pid()`。这些条件决定检查、重试或提前返回路径。
@note 源码中出现的锁操作：`state_lock_.acquire` → `state_lock_.release`。顺序包含分支和释放动作，不表示这些锁同时持有。
@pre 等待路径必须遵守“条件锁下登记、释放条件锁后真正睡眠”的协议；不能携带无关自旋锁进入调度。是否可中断由具体登记入口决定。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-4bf66df4f890a6a5"></a>

## `xv::sync::SleepLock::acquire_interruptible`

```doxygen
@fn bool xv::sync::SleepLock::acquire_interruptible()
@brief 睡眠锁：等待锁可用，并允许可投递信号取消。
@details 与普通睡眠锁等待采用相同登记协议，但在每轮尝试前检查 interrupted，并使用可中断通道。取消时返回 false；成功时调用者负责配对 release。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`state_lock_.acquire` → `detail::interrupted` → `state_lock_.release` → `detail::current_pid` → `detail::fail` → `detail::prepare_interruptible_sleep` → `detail::sleep_process`。
@details 本函数可见的状态更新包括：`locked_ = true;`；`pid_ = detail::current_pid();`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`detail::interrupted()`；`!locked_`；`pid_ == detail::current_pid()`。这些条件决定检查、重试或提前返回路径。
@note 源码中出现的锁操作：`state_lock_.acquire` → `state_lock_.release`。顺序包含分支和释放动作，不表示这些锁同时持有。
@pre 等待路径必须遵守“条件锁下登记、释放条件锁后真正睡眠”的协议；不能携带无关自旋锁进入调度。是否可中断由具体登记入口决定。
@return 各分支实际返回：`false`；`true`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-cb0bae526473552b"></a>

## `xv::sync::SleepLock::available`

```doxygen
@fn bool xv::sync::SleepLock::available(PollTable *table) const
@brief 睡眠锁：检查资源是否可用，并按参数登记等待依赖。
@param table 页表或 poll 登记接口；其具体类型见函数签名。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`guard` → `table->watch`。
@details 需要特别跟踪的条件：`table`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`!locked_`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-e399786b3d75952b"></a>

## `xv::sync::SleepLock::try_acquire`

```doxygen
@fn bool xv::sync::SleepLock::try_acquire()
@brief 睡眠锁：尝试取得锁，失败时不睡眠。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`guard` → `detail::current_pid`。
@details 本函数可见的状态更新包括：`locked_ = true;`；`pid_ = detail::current_pid();`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`locked_`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`false`；`true`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-9c53ea1cb256794a"></a>

## `xv::sync::SleepLock::release`

```doxygen
@fn void xv::sync::SleepLock::release()
@brief 睡眠锁：释放当前引用或归还对象占用的资源。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`guard` → `detail::current_pid` → `detail::fail` → `detail::wake_processes`。
@details 本函数可见的状态更新包括：`locked_ = false;`；`pid_ = 0;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`!locked_ || pid_ != detail::current_pid()`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-d8314f38391b20f3"></a>

## `xv::sync::SleepLock::holding`

```doxygen
@fn bool xv::sync::SleepLock::holding() const
@brief 睡眠锁：检查当前执行者是否拥有此锁。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`guard` → `detail::current_pid`。
@return 各分支实际返回：`locked_ && pid_ == detail::current_pid()`。应结合上面的条件检查区分正常值、空值和错误值。
```
