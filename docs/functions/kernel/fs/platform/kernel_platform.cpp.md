# `kernel/fs/platform/kernel_platform.cpp`

开发职责：文件描述、inode、路径、块缓存与日志事务。

源码：[打开实际文件](../../../../../kernel/fs/platform/kernel_platform.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.fs`。

直接模块依赖：`xv.print`, `xv.runtime`, `:platform`, `xv.process`, `xv.memory`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::fs::detail::allocate_page`](kernel_platform.cpp.md#fn-ac353f7455a3e838)
- [`xv::fs::detail::compare`](kernel_platform.cpp.md#fn-2157cfabe1773e44)
- [`xv::fs::detail::copy_fixed`](kernel_platform.cpp.md#fn-44100aabfc8b85fc)
- [`xv::fs::detail::copy_in`](kernel_platform.cpp.md#fn-73ff94af3db47c5c)
- [`xv::fs::detail::copy_out`](kernel_platform.cpp.md#fn-092f21133e90643a)
- [`xv::fs::detail::current_directory`](kernel_platform.cpp.md#fn-f7b6f6a96cd5d89c)
- [`xv::fs::detail::current_pid`](kernel_platform.cpp.md#fn-89d386033a15eb3c)
- [`xv::fs::detail::descriptor_snapshot`](kernel_platform.cpp.md#fn-2eb27cd7b4d7c8ad)
- [`xv::fs::detail::fail`](kernel_platform.cpp.md#fn-15b1609ad1be02bf)
- [`xv::fs::detail::fill`](kernel_platform.cpp.md#fn-1f96e9ad94b54ca8)
- [`xv::fs::detail::free_page`](kernel_platform.cpp.md#fn-2edb4be43f545533)
- [`xv::fs::detail::identity`](kernel_platform.cpp.md#fn-64fd9fffd257e7c3)
- [`xv::fs::detail::killed`](kernel_platform.cpp.md#fn-a4b206516ef7c6e3)
- [`xv::fs::detail::memory_info`](kernel_platform.cpp.md#fn-e2f78f5fd9feb4a0)
- [`xv::fs::detail::move`](kernel_platform.cpp.md#fn-7edb69559eaac2a2)
- [`xv::fs::detail::prepare_interruptible_sleep`](kernel_platform.cpp.md#fn-fcf4728d931b520f)
- [`xv::fs::detail::prepare_sleep`](kernel_platform.cpp.md#fn-ab640082c3d8f293)
- [`xv::fs::detail::process_after`](kernel_platform.cpp.md#fn-d36c15ba7da69bbe)
- [`xv::fs::detail::report`](kernel_platform.cpp.md#fn-f79daff3187ea060)
- [`xv::fs::detail::sleep`](kernel_platform.cpp.md#fn-cc457fade9ae62c6)
- [`xv::fs::detail::uptime_ticks`](kernel_platform.cpp.md#fn-0d0decfcd0e63551)
- [`xv::fs::detail::wakeup`](kernel_platform.cpp.md#fn-aec05456dbae85c7)

<a id="fn-fcf4728d931b520f"></a>

## `xv::fs::detail::prepare_interruptible_sleep`

```doxygen
@fn void xv::fs::detail::prepare_interruptible_sleep(void *channel)
@brief 文件描述、inode、路径、块缓存与日志事务：登记可中断的睡眠意图。
@param channel 等待条件或通知对象的地址键；不能在未结束登记时销毁对应对象。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::prepare_interruptible_sleep`。
@pre 等待路径必须遵守“条件锁下登记、释放条件锁后真正睡眠”的协议；不能携带无关自旋锁进入调度。是否可中断由具体登记入口决定。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-ab640082c3d8f293"></a>

## `xv::fs::detail::prepare_sleep`

```doxygen
@fn void xv::fs::detail::prepare_sleep(void *channel)
@brief 文件描述、inode、路径、块缓存与日志事务：登记不可中断的睡眠意图。
@param channel 等待条件或通知对象的地址键；不能在未结束登记时销毁对应对象。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::prepare_sleep`。
@pre 等待路径必须遵守“条件锁下登记、释放条件锁后真正睡眠”的协议；不能携带无关自旋锁进入调度。是否可中断由具体登记入口决定。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-cc457fade9ae62c6"></a>

## `xv::fs::detail::sleep`

```doxygen
@fn void xv::fs::detail::sleep()
@brief 文件描述、inode、路径、块缓存与日志事务：在已登记的等待条件下挂起当前进程。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::sleep`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-aec05456dbae85c7"></a>

## `xv::fs::detail::wakeup`

```doxygen
@fn void xv::fs::detail::wakeup(void *channel)
@brief 文件描述、inode、路径、块缓存与日志事务：把通道唤醒请求交给进程等待服务。
@param channel 等待条件或通知对象的地址键；不能在未结束登记时销毁对应对象。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::wakeup`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-a4b206516ef7c6e3"></a>

## `xv::fs::detail::killed`

```doxygen
@fn bool xv::fs::detail::killed()
@brief 文件描述、inode、路径、块缓存与日志事务：读取或计算 `xv::process::current()->is_killed()`，返回当前查询结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::current` → `is_killed`。
@return 各分支实际返回：`xv::process::current()->is_killed()`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-f7b6f6a96cd5d89c"></a>

## `xv::fs::detail::current_directory`

```doxygen
@fn Inode *xv::fs::detail::current_directory()
@brief 文件描述、inode、路径、块缓存与日志事务：读取或计算 `xv::process::current()->directory().duplicate()`，返回当前查询结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::current` → `directory` → `duplicate`。
@return 各分支实际返回：`xv::process::current()->directory().duplicate()`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-73ff94af3db47c5c"></a>

## `xv::fs::detail::copy_in`

```doxygen
@fn int xv::fs::detail::copy_in(void *destination, bool user, unsigned long source, unsigned count)
@brief 文件描述、inode、路径、块缓存与日志事务：从指定地址空间复制数据到内核缓冲。
@param destination 接收结果的缓冲或目标对象。
@param user 指明地址是否属于用户地址空间。
@param source 输入数据位置或源对象。
@param count 本次操作的数量上限。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::copy_in`。
@note 此路径涉及用户地址与内核地址的边界。复制失败必须按当前分支退出，不能继续把用户地址作为普通内核指针使用。
@return 各分支实际返回：`xv::process::copy_in(destination, user, source, count)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-092f21133e90643a"></a>

## `xv::fs::detail::copy_out`

```doxygen
@fn int xv::fs::detail::copy_out(bool user, unsigned long destination, const void *source, unsigned count)
@brief 文件描述、inode、路径、块缓存与日志事务：把内核缓冲复制到指定地址空间。
@param user 指明地址是否属于用户地址空间。
@param destination 接收结果的缓冲或目标对象。
@param source 输入数据位置或源对象。
@param count 本次操作的数量上限。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::copy_out`。
@note 此路径涉及用户地址与内核地址的边界。复制失败必须按当前分支退出，不能继续把用户地址作为普通内核指针使用。
@return 各分支实际返回：`xv::process::copy_out(user, destination, source, count)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-ac353f7455a3e838"></a>

## `xv::fs::detail::allocate_page`

```doxygen
@fn void *xv::fs::detail::allocate_page()
@brief 文件描述、inode、路径、块缓存与日志事务：取得一个物理页或内部节点页。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::memory::allocator` → `allocate`。
@return 各分支实际返回：`xv::memory::allocator().allocate()`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-2edb4be43f545533"></a>

## `xv::fs::detail::free_page`

```doxygen
@fn void xv::fs::detail::free_page(void *page)
@brief 文件描述、inode、路径、块缓存与日志事务：归还先前取得的页面。
@param page 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::memory::allocator` → `deallocate`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-7edb69559eaac2a2"></a>

## `xv::fs::detail::move`

```doxygen
@fn void xv::fs::detail::move(void *destination, const void *source, unsigned count)
@brief 文件描述、inode、路径、块缓存与日志事务：移动字节区域并允许源目标重叠。
@param destination 接收结果的缓冲或目标对象。
@param source 输入数据位置或源对象。
@param count 本次操作的数量上限。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::runtime::move`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-1f96e9ad94b54ca8"></a>

## `xv::fs::detail::fill`

```doxygen
@fn void xv::fs::detail::fill(void *destination, int value, unsigned count)
@brief 文件描述、inode、路径、块缓存与日志事务：把字节区域填充为指定值。
@param destination 接收结果的缓冲或目标对象。
@param value 本次操作使用的 value 值。
@param count 本次操作的数量上限。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::runtime::fill`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-2157cfabe1773e44"></a>

## `xv::fs::detail::compare`

```doxygen
@fn int xv::fs::detail::compare(const char *first, const char *second, unsigned count)
@brief 文件描述、inode、路径、块缓存与日志事务：按长度比较两个字节区域。
@param first 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param second 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param count 本次操作的数量上限。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::runtime::compare_text`。
@return 各分支实际返回：`xv::runtime::compare_text(first, second, count)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-44100aabfc8b85fc"></a>

## `xv::fs::detail::copy_fixed`

```doxygen
@fn void xv::fs::detail::copy_fixed(char *destination, const char *source, unsigned count)
@brief 文件描述、inode、路径、块缓存与日志事务：复制固定宽度字符串字段。
@param destination 接收结果的缓冲或目标对象。
@param source 输入数据位置或源对象。
@param count 本次操作的数量上限。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::runtime::copy_fixed`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-f79daff3187ea060"></a>

## `xv::fs::detail::report`

```doxygen
@fn void xv::fs::detail::report(const char *message, int first, int second)
@brief 文件描述、inode、路径、块缓存与日志事务：report。
@param message 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param first 本次操作使用的 首项 值。
@param second 本次操作使用的 second 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::print::printk`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-15b1609ad1be02bf"></a>

## `xv::fs::detail::fail`

```doxygen
@fn void xv::fs::detail::fail(const char *message)
@brief 文件描述、inode、路径、块缓存与日志事务：报告内核不变量错误并停止正常执行。
@param message 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::print::panic`。
@return 不返回正常调用点；后续执行流由异常终止、调度或特权级切换接管。
```
<a id="fn-d36c15ba7da69bbe"></a>

## `xv::fs::detail::process_after`

```doxygen
@fn bool xv::fs::detail::process_after(int after, ProcessRecord &result)
@brief 复制下一个 PID 的进程与凭据信息。
@details 通过进程快照服务取得 uid/euid/gid/egid 和原有调度、内存信息；失败不发布记录，procfs 不保留活进程对象。
```

<a id="fn-0d0decfcd0e63551"></a>

## `xv::fs::detail::uptime_ticks`

```doxygen
@fn unsigned long xv::fs::detail::uptime_ticks()
@brief 读取系统运行时钟的 tick 快照。
@details 委托 TickClock::uptime，以系统时钟锁同步；QEMU virt 的当前间隔为 100 ms，格式化由 procfs 完成。
```

<a id="fn-e2f78f5fd9feb4a0"></a>

## `xv::fs::detail::memory_info`

```doxygen
@fn void xv::fs::detail::memory_info(unsigned long &total_kib, unsigned long &free_kib)
@brief 取得分配器管理总页数与空闲页数，换算为 KiB。
@details 总量为所有 usable 区间页数之和，排除内核、保留区与分配器元数据；空闲量来自原子计数。该指标不代表进程虚拟地址空间总和。
```
<a id="fn-89d386033a15eb3c"></a>

## `xv::fs::detail::current_pid`

```doxygen
@fn int xv::fs::detail::current_pid()
@brief 取得执行当前文件系统操作的进程 PID。
@details 仅在已调度的进程上下文调用，用于 /proc/self 的查找时解析；不保留 Process 指针。
```

<a id="fn-2eb27cd7b4d7c8ad"></a>

## `xv::fs::detail::descriptor_snapshot`

```doxygen
@fn int xv::fs::detail::descriptor_snapshot(int pid, DescriptorRecord *records, unsigned capacity)
@brief 取得目标进程已发布描述符的复制信息。
@details 使用进程服务在进程锁和短暂描述符表锁内复制 FileInfo，随后筛选有效 identity 并附加 fd 号。全程不调用后端、不取得节点睡眠锁、不释放文件引用，避免观测路径触发最后关闭和嵌套日志事务。
```
<a id="fn-64fd9fffd257e7c3"></a>

## `xv::fs::detail::identity`

```doxygen
@fn Identity xv::fs::detail::identity(bool real)
@brief 复制当前进程的文件系统身份。
@details real 参数选择真实或有效 uid/gid，附加组及 umask 从同一份凭据快照复制；不向文件系统暴露 Process 指针。
```
