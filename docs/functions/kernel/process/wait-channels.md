# 等待通道：登记、撤销与按对象唤醒

本文是独立的函数级开发说明；Doxygen 内容由工具写入 clangd 外部索引，不注入源码。

<a id="wait-waitchannels-index"></a>

## `xv::process::WaitChannels::index`

```doxygen
@fn unsigned xv::process::WaitChannels::index(void *channel)
@brief 把等待通道地址映射到固定桶号；散列碰撞不代表通道相同。
@param channel 仅作为身份键使用的通道地址，不解引用指向内容。
@return 0 到 BucketCount-1 的桶下标。
@details 将地址分别右移 3 位和 11 位后异或，再对 128 取模。它减少低位对齐导致的集中；真正唤醒时仍比较完整通道地址。
@pre 无须持锁；同一地址必须始终得到同一桶。
@note 桶数限制查找碰撞，不限制进程数量或 poll 订阅数量。
@see WaitChannels::wake
```

<a id="wait-waitchannels-initialize"></a>

## `xv::process::WaitChannels::initialize`

```doxygen
@fn void xv::process::WaitChannels::initialize()
@brief 初始化等待通道桶锁和 poll 超时链表锁。
@return 无返回值。
@pre 启动期间调用，等待表还没有被并发使用。
@details timer_lock_ 保护有限超时 poll 链表；每个 bucket.lock 分别保护普通等待者和 poll 订阅链。构造时已将链表头初始化为空。
@note 初始化只建立锁状态，不分配每进程等待节点。普通等待节点嵌入 Process，poll 节点由每次 PollWait 管理。
```

<a id="wait-waitchannels-unlink"></a>

## `xv::process::WaitChannels::unlink`

```doxygen
@fn void xv::process::WaitChannels::unlink(Bucket &bucket, Process &process)
@brief 从一个桶撤销进程的普通等待登记，并清除睡眠意图。
@param bucket 当前登记所在的桶。
@param process 必须已存在于 bucket.head 链表中的进程。
@return 无返回值。
@pre 调用者同时持有 bucket.lock 与 process.lock。
@details 用指向链接的指针定位进程，摘除后清空 wait_next_，以 release 原子存储清空 wait_key_，再清空 chan 与 interruptible_。
@note 找不到节点属于内核登记不变量被破坏，会调用 storage_fail；它不会自行把进程发布到就绪队列。
@see WaitChannels::cancel
@see WaitChannels::wake
```

<a id="wait-waitchannels-cancel"></a>

## `xv::process::WaitChannels::cancel`

```doxygen
@fn void xv::process::WaitChannels::cancel(Process &process)
@brief 安全撤销进程遗留的普通通道登记。
@param process 需要清理登记、且在本次调用期间保持存活的进程。
@return 无返回值；没有登记时立即返回。
@pre 调用者不能已持有 process.lock 或将要获取的桶锁。
@details 先以 acquire 原子读取 wait_key_；再按桶锁、进程锁的顺序加锁，重新核对键值。如果并发唤醒或重新登记已改变键，则释放锁后重试；仍相同时交给 unlink。
@note 信号唤醒可以只清除 chan，暂时保留桶节点。本函数负责之后真正摘链，防止节点悬空；不清理独立的 poll 订阅。
@see WaitChannels::prepare
@details 流程由 CancelFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="wait-waitchannels-prepare"></a>

## `xv::process::WaitChannels::prepare`

```doxygen
@fn void xv::process::WaitChannels::prepare(Process &process, void *channel, bool interruptible)
@brief 登记睡眠意图，让条件检查到真正睡眠之间的通知不会丢失。
@param process 将要等待的进程，通常为 current()。
@param channel 非空对象地址，作为等待条件的身份键；零地址触发内核失败。
@param interruptible 为 true 时，可投递信号或旧 killed 标志能够阻止本次睡眠。
@return 无返回值；是否真正睡眠由后续 sleep 检查 chan 决定。
@pre 调用者通常持有保护等待条件的锁，但不能已持有 process.lock 或目标桶锁。
@details 先 cancel 旧登记，再按桶锁、进程锁加锁。可中断且已有待处理事件时清空 chan 并返回；否则设置 interruptible_、chan 和 wait_next_，插入桶链，最后 release 发布 wait_key_。
@details 调用者接着释放条件锁并调用 sleep。提前发生的 wake 会摘链并清空 chan；sleep 在进程锁下看到 chan 为空便跳过调度。因此 prepare 本身不切换 CPU，也不把状态改为 Sleeping。
@note 被屏蔽或忽略的信号不会触发 interrupting。捕获信号会设置 syscall_interrupted_，供系统调用分发器决定 EINTR 或 SA_RESTART。
@see xv::process::prepare_interruptible_sleep
@see xv::process::sleep
@see WaitChannels::wake
@details 流程由 PrepareFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="wait-waitchannels-wake"></a>

## `xv::process::WaitChannels::wake`

```doxygen
@fn void xv::process::WaitChannels::wake(void *channel)
@brief 唤醒指定对象通道的普通等待者，并通知该通道的 poll 订阅。
@param channel 完整通道地址；为空时不执行任何操作。
@return 无返回值，允许没有等待者。
@pre 可以在不能睡眠的上下文调用；调用者不能已持有目标桶锁或将被通知的进程锁。
@details 获取桶锁后遍历普通链，只处理 wait_key_ 与 channel 完整相等的进程。取得对应进程锁，先 unlink；只有状态已为 Sleeping 时才调用 scheduler_.make_runnable。
@details 对还没真正睡眠的进程，清空 chan 已足够。随后遍历同桶 pollers，按完整地址通知 PollWait；poll 回调在进程锁下记录 triggered，防止通知早于睡眠被遗忘。
@note 哈希碰撞不会误唤醒其他对象；节点先保存 next 再摘除。全过程不执行用户处理函数，也不直接切换到被唤醒进程。
@see WaitChannels::prepare
@details 流程由 WakeFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```
