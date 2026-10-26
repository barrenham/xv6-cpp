# `kernel/process/wait/wait_child.cpp`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../kernel/process/wait/wait_child.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process`。

直接模块依赖：`:manager`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::process::ProcessManager::wait`](wait_child.cpp.md#fn-15b2744ee3edc5d8)
- [`xv::process::ProcessManager::wait_child`](wait_child.cpp.md#fn-d7ca44eff96231ec)
- [`xv::process::ProcessManager::waitpid`](wait_child.cpp.md#fn-3d4e53d945a90f89)

<a id="fn-15b2744ee3edc5d8"></a>

## `xv::process::ProcessManager::wait`

```doxygen
@fn int xv::process::ProcessManager::wait(unsigned long address)
@brief 进程生命周期管理器：读取或计算 `wait_child(-1, address, 0, false)`，返回当前查询结果。
@param address 地址参数；用户地址必须通过内存复制/映射校验路径访问。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`wait_child`。
@return 各分支实际返回：`wait_child(-1, address, 0, false)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-3d4e53d945a90f89"></a>

## `xv::process::ProcessManager::waitpid`

```doxygen
@fn int xv::process::ProcessManager::waitpid(int pid, unsigned long address, int options)
@brief 进程生命周期管理器：读取或计算 `-1`，返回当前查询结果。
@param pid 进程选择编号；特殊值语义由对应进程接口决定。
@param address 地址参数；用户地址必须通过内存复制/映射校验路径访问。
@param options 调用选项位组合。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`wait_child`。
@details 需要特别跟踪的条件：`(options & ~(WNOHANG | WUNTRACED | WCONTINUED)) || pid < -0x7fffffff`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`-1`；`wait_child(pid, address, options, true)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-d7ca44eff96231ec"></a>

## `xv::process::ProcessManager::wait_child`

```doxygen
@fn int xv::process::ProcessManager::wait_child(int selector, unsigned long address, int options, bool encoded)
@brief 进程生命周期管理器：等待child。
@param selector 本次操作使用的 selector 值。
@param address 地址参数；用户地址必须通过内存复制/映射校验路径访问。
@param options 调用选项位组合。
@param encoded 本次操作使用的 encoded 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`current` → `groups_.get_group` → `wait_lock_.acquire` → `Child` → `parent_` → `pid_` → `group_` → `options_` → `groups_` → `matches` → `match` → `store_.find`。
@details 本函数可见的状态更新包括：`child->stop_event_ = false;`；`child->continued_event_ = false;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`child.state == ProcessState::Unused || child.parent != parent_ || (pid_ > 0 && child.pid != pid_) || (group_ && groups_.get_group(child.membership_) != group_)`；`auto *child = store_.find(match)`；`address && parent->memory_.copy_out(address, &status, sizeof(status)) < 0`；`exited`；`WIFSTOPPED(status)`。这些条件决定检查、重试或提前返回路径。
@note 源码中出现的锁操作：`wait_lock_.acquire` → `child->lock.release` → `wait_lock_.release`。顺序包含分支和释放动作，不表示这些锁同时持有。
@pre 等待路径必须遵守“条件锁下登记、释放条件锁后真正睡眠”的协议；不能携带无关自旋锁进入调度。是否可中断由具体登记入口决定。
@note 此路径涉及用户地址与内核地址的边界。复制失败必须按当前分支退出，不能继续把用户地址作为普通内核指针使用。
@return 各分支实际返回：`false`；`child.state == ProcessState::Zombie || ((options_ & WUNTRACED) && child.stop_event_) || ((options_ & WCONTINUED) && child.continued_event_)`；`-1`；`pid`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
```
