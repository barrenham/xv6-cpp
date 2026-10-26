# `kernel/fs/journal/journal.cpp`

开发职责：文件描述、inode、路径、块缓存与日志事务。

源码：[打开实际文件](../../../../../kernel/fs/journal/journal.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.fs`。

直接模块依赖：`:platform`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::fs::Journal::begin`](journal.cpp.md#fn-68275ab1a63eaf6b)
- [`xv::fs::Journal::commit`](journal.cpp.md#fn-61fed5e757572a19)
- [`xv::fs::Journal::end`](journal.cpp.md#fn-9a8ba69510b2c5fb)
- [`xv::fs::Journal::initialize`](journal.cpp.md#fn-632dbb783eedc4e0)
- [`xv::fs::Journal::install`](journal.cpp.md#fn-a2705e4cf85be2f4)
- [`xv::fs::Journal::read_header`](journal.cpp.md#fn-d538c5309149d8cd)
- [`xv::fs::Journal::record`](journal.cpp.md#fn-2bff0beebb0a1b13)
- [`xv::fs::Journal::recover`](journal.cpp.md#fn-a46bb87dec6e1fcb)
- [`xv::fs::Journal::sync`](journal.cpp.md#fn-5798daa238e10eb6)
- [`xv::fs::Journal::write_header`](journal.cpp.md#fn-95f9d303c4dca3ed)
- [`xv::fs::Journal::write_log`](journal.cpp.md#fn-0cdba2dc483b4403)

<a id="fn-632dbb783eedc4e0"></a>

## `xv::fs::Journal::initialize`

```doxygen
@fn void xv::fs::Journal::initialize(int dev, const superblock *sb)
@brief 文件系统重做日志：建立该对象开始工作所需的初始状态。
@param dev 块设备编号。
@param sb 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`lock_.init` → `recover`。
@details 本函数可见的状态更新包括：`start_ = sb->logstart;`；`device_ = dev;`。局部临时变量与对象字段应按作用域区分。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-a2705e4cf85be2f4"></a>

## `xv::fs::Journal::install`

```doxygen
@fn void xv::fs::Journal::install(int recovering)
@brief 文件系统重做日志：把已提交日志安装到原始数据块位置。
@param recovering 本次操作使用的 recovering 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`detail::report` → `cache_.read` → `detail::move` → `dbuf->bytes` → `lbuf->bytes` → `cache_.write` → `cache_.unpin` → `lbuf.reset` → `dbuf.reset`。
@details 需要特别跟踪的条件：`recovering`；`recovering == 0`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-d538c5309149d8cd"></a>

## `xv::fs::Journal::read_header`

```doxygen
@fn void xv::fs::Journal::read_header(void)
@brief 文件系统重做日志：从磁盘恢复日志头记录。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`cache_.read` → `buf->bytes` → `buf.reset`。
@details 本函数可见的状态更新包括：`header_.count = lh->count;`；`header_.blocks[i] = lh->blocks[i];`。局部临时变量与对象字段应按作用域区分。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-95f9d303c4dca3ed"></a>

## `xv::fs::Journal::write_header`

```doxygen
@fn void xv::fs::Journal::write_header(void)
@brief 文件系统重做日志：写入作为提交点的磁盘日志头。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`cache_.read` → `buf->bytes` → `cache_.write` → `buf.reset`。
@details 本函数可见的状态更新包括：`hb->count = header_.count;`；`hb->blocks[i] = header_.blocks[i];`。局部临时变量与对象字段应按作用域区分。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-a46bb87dec6e1fcb"></a>

## `xv::fs::Journal::recover`

```doxygen
@fn void xv::fs::Journal::recover(void)
@brief 文件系统重做日志：重放已提交日志并清空提交记录。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`read_header` → `install` → `write_header`。
@details 本函数可见的状态更新包括：`header_.count = 0;`。局部临时变量与对象字段应按作用域区分。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-68275ab1a63eaf6b"></a>

## `xv::fs::Journal::begin`

```doxygen
@fn void xv::fs::Journal::begin(void)
@brief 文件系统重做日志：按日志空间预留规则进入事务。
@details 持日志锁检查是否正在提交以及为 outstanding 操作预留的空间是否足够；不足时登记等待后释放锁睡眠。准入后增加 outstanding，防止事务中途才发现日志预留不足。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`lock_.acquire` → `detail::prepare_sleep` → `lock_.release` → `detail::sleep`。
@details 本函数可见的状态更新包括：`outstanding_ += 1;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`1`；`committing_`；`header_.count + (outstanding_ + 1) * MAXOPBLOCKS > LOGBLOCKS`。这些条件决定检查、重试或提前返回路径。
@note 源码中出现的锁操作：`lock_.acquire` → `lock_.release`。顺序包含分支和释放动作，不表示这些锁同时持有。
@pre 等待路径必须遵守“条件锁下登记、释放条件锁后真正睡眠”的协议；不能携带无关自旋锁进入调度。是否可中断由具体登记入口决定。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-9a8ba69510b2c5fb"></a>

## `xv::fs::Journal::end`

```doxygen
@fn void xv::fs::Journal::end(void)
@brief 文件系统重做日志：结束本次事务，并在最后一个操作结束时提交。
@details 减少 outstanding；最后一个操作置 committing 后在不持日志锁的情况下提交。提交完成后清标志、增加提交计数并唤醒准入者；仍有操作时也通知因预留空间不足而等待的调用者。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`lock_.acquire` → `detail::fail` → `detail::wakeup` → `lock_.release` → `commit`。
@details 本函数可见的状态更新包括：`outstanding_ -= 1;`；`committing_ = 1;`；`committing_ = 0;`；`commits_ += 1;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`committing_`；`outstanding_ == 0`；`do_commit`。这些条件决定检查、重试或提前返回路径。
@note 源码中出现的锁操作：`lock_.acquire` → `lock_.release`。顺序包含分支和释放动作，不表示这些锁同时持有。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-0cdba2dc483b4403"></a>

## `xv::fs::Journal::write_log`

```doxygen
@fn void xv::fs::Journal::write_log(void)
@brief 文件系统重做日志：把被修改的缓存块复制到日志区。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`cache_.read` → `detail::move` → `to->bytes` → `from->bytes` → `cache_.write` → `from.reset` → `to.reset`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-61fed5e757572a19"></a>

## `xv::fs::Journal::commit`

```doxygen
@fn void xv::fs::Journal::commit()
@brief 文件系统重做日志：把重做日志按提交协议持久化。
@details 按 write_log → write_header → install → 清空内存日志头 → write_header 的次序提交。写入非空日志头是提交点；之后即使重启也可通过 recover 重放到原始块。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`write_log` → `write_header` → `install`。
@details 本函数可见的状态更新包括：`header_.count = 0;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`header_.count > 0`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-2bff0beebb0a1b13"></a>

## `xv::fs::Journal::record`

```doxygen
@fn void xv::fs::Journal::record(Buffer &buffer)
@brief 文件系统重做日志：登记本次需要持久化的块或统计记录。
@param buffer 接收或提交的数据缓冲。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`lock_.acquire` → `detail::fail` → `b->block_number` → `cache_.pin` → `lock_.release`。
@details 本函数可见的状态更新包括：`header_.blocks[i] = b->block_number();`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`header_.count >= LOGBLOCKS`；`outstanding_ < 1`；`header_.blocks[i] == b->block_number()`；`i == header_.count`。这些条件决定检查、重试或提前返回路径。
@note 源码中出现的锁操作：`lock_.acquire` → `lock_.release`。顺序包含分支和释放动作，不表示这些锁同时持有。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-5798daa238e10eb6"></a>

## `xv::fs::Journal::sync`

```doxygen
@fn void xv::fs::Journal::sync(void)
@brief 文件系统重做日志：等待或提交文件系统当前修改。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`lock_.acquire` → `detail::prepare_sleep` → `lock_.release` → `detail::sleep`。
@details 需要特别跟踪的条件：`committing_ || outstanding_ > 0`；`commits_ < n`。这些条件决定检查、重试或提前返回路径。
@note 源码中出现的锁操作：`lock_.acquire` → `lock_.release`。顺序包含分支和释放动作，不表示这些锁同时持有。
@pre 等待路径必须遵守“条件锁下登记、释放条件锁后真正睡眠”的协议；不能携带无关自旋锁进入调度。是否可中断由具体登记入口决定。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```
