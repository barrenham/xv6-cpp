# 时钟计数与进程等待

本文是独立的函数级开发说明；Doxygen 内容由工具写入 clangd 外部索引，不注入源码。

<a id="clock-tickclock-initialize"></a>

## `xv::trap::TickClock::initialize`

```doxygen
@fn void xv::trap::TickClock::initialize()
@brief 初始化全局 tick 计数所需的锁。
@return 无返回值。
@pre 时钟服务尚未被中断和进程并发访问。
@note 计数初值由 TickClock 构造参数决定；本函数不修改 ticks_，也不装载下一次硬件中断。
```

<a id="clock-tickclock-interrupt"></a>

## `xv::trap::TickClock::interrupt`

```doxygen
@fn void xv::trap::TickClock::interrupt()
@brief 处理本 hart 的定时事件，在 hart 0 增加全局计数并唤醒 tick 等待者。
@return 无返回值。
@pre 由内核定时中断分发路径调用，不允许睡眠。
@details hart 0 持 lock_ 更新 ticks_，按 &ticks_ 通道唤醒等待者。每个 hart 都以当前硬件时间加 TimerInterval 重新装载自己的下一次中断。
@note TimerInterval=1000000；当前 QEMU virt 时基下约 100ms。其他 hart 不重复增加全局 ticks，避免 SMP 下计时倍增。
@see TickClock::pause
```

<a id="clock-tickclock-uptime"></a>

## `xv::trap::TickClock::uptime`

```doxygen
@fn unsigned xv::trap::TickClock::uptime() const
@brief 在锁保护下读取全局 tick 计数。
@return ticks_ 的无符号快照；不是毫秒数。
@details 使用可变成员 lock_ 保护只读访问，与 hart 0 的递增互斥。调用不会睡眠。
@note 计数允许无符号回绕；等待时间比较采用差值，而不是简单比较绝对截止值。
```

<a id="clock-tickclock-pause"></a>

## `xv::trap::TickClock::pause`

```doxygen
@fn int xv::trap::TickClock::pause(int count)
@brief 让当前进程按全局 tick 计数可中断地等待。
@param count 请求等待的 tick 数；负值按零处理。
@return 等待完成返回 0；被可投递信号或 killed 中断时返回 -1，由系统调用层进一步决定 EINTR 或重启。
@pre 必须有当前进程；进入时不持有本时钟锁或其他不允许跨等待保留的锁。
@details 取得 lock_ 并记录起始 ticks_，用无符号差值检查经过时间。未到期时先检查 interrupted，再在 &ticks_ 上登记可中断睡眠；释放时钟锁后 sleep，恢复后重新加锁并重查。
@note 条件检查与等待登记之间一直持有时钟锁，因此 interrupt 的递增/唤醒不能越过登记窗口。真正调度时只持进程锁，不持时钟锁。
@see xv::process::prepare_interruptible_sleep
@see xv::process::sleep
@see TickClock::interrupt
@details 编排由 PauseFlow 的模板图声明；具名阶段负责业务操作，控制策略负责下一节点。原有等待协议、引用所有权和错误返回约定保留。
```
