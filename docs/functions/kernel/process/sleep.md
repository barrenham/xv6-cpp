# 当前进程：登记后睡眠

本文是独立的函数级开发说明；Doxygen 内容由工具写入 clangd 外部索引，不注入源码。

<a id="sleep-prepare-sleep"></a>

## `xv::process::prepare_sleep`

```doxygen
@fn void xv::process::prepare_sleep(void *channel)
@brief 为当前进程登记不可中断的普通通道等待。
@param channel 非空等待条件地址，例如磁盘请求或内部锁对象。
@return 无返回值；调用后仍处于当前执行上下文。
@pre 按等待条件协议持有条件锁，调用后释放它，再调用 sleep。
@details 把 current() 和通道转交给 WaitChannels::prepare，使用默认 interruptible=false。此包装隐藏全局等待表，条件锁不会由本函数自动释放。
@note 磁盘、日志等协议可能必须完成，即使有信号也不能随意放弃。必须在循环中重查条件，不能把一次唤醒当作条件已满足。
@see xv::process::sleep
```

<a id="sleep-prepare-interruptible-sleep"></a>

## `xv::process::prepare_interruptible_sleep`

```doxygen
@fn void xv::process::prepare_interruptible_sleep(void *channel)
@brief 为当前进程登记可由信号取消的等待。
@param channel 非空等待条件地址，如 TTY 输入、管道数据、子状态事件或 tick 计数地址。
@return 无返回值；登记可能因已有信号而被跳过。
@pre 调用者先在条件锁下检查条件，再调用本函数；释放条件锁后调用 sleep。
@details 转交 WaitChannels::prepare(current, channel, true)。最终信号检查与登记在同一进程锁下完成；发送者也按进程锁修改睡眠意图，关闭检查后立刻收到信号的竞态窗口。
@note 恢复后先重新取得条件锁并重查条件与中断状态，逐层释放资源，之后 trap 返回用户态前才执行信号动作。
@see xv::process::prepare_sleep
@see xv::process::sleep
```

<a id="sleep-sleep"></a>

## `xv::process::sleep`

```doxygen
@fn void xv::process::sleep()
@brief 依据已登记的 chan 决定是否挂起当前进程。
@return 无返回值；返回时进程已重新获得 CPU，或者因提前通知根本没有睡眠。
@pre 必须先调用 prepare_sleep 或 prepare_interruptible_sleep，并释放所有条件锁。进入时不能持有 current()->lock。
@details 取得当前进程锁；chan 非空时将状态改为 Sleeping 并调用 scheduler().suspend。只有该进程锁跨上下文切换，恢复后释放它。chan 已为空则直接释放锁返回。
@note 通知发生在 prepare 与本函数之间时，chan 已被清空，不会错过唤醒。本函数不保证业务条件成立，调用者必须用循环检查条件。
@see WaitChannels::prepare
@see WaitChannels::wake
```
