# `kernel/net/platform/kernel_platform.cpp`

## 函数导航

- [`xv::net::detail::allocate_page`](kernel_platform.cpp.md#fn-a39640ebe645b6ba)
- [`xv::net::detail::copy_in`](kernel_platform.cpp.md#fn-d96c58b6fbc16587)
- [`xv::net::detail::copy_out`](kernel_platform.cpp.md#fn-073e4679a830e329)
- [`xv::net::detail::free_page`](kernel_platform.cpp.md#fn-9ef32a99d26fe40e)
- [`xv::net::detail::interrupted`](kernel_platform.cpp.md#fn-99f626cb5d17f655)
- [`xv::net::detail::owner`](kernel_platform.cpp.md#fn-7079e964eeda41e2)
- [`xv::net::detail::prepare_sleep`](kernel_platform.cpp.md#fn-eeff613785c4e3e3)
- [`xv::net::detail::sleep`](kernel_platform.cpp.md#fn-b5609c0fae83d6c4)
- [`xv::net::detail::wake`](kernel_platform.cpp.md#fn-44622dca17b535c2)


独立 Doxygen 开发文档；运行代码位于 `kernel/`。


<a id="fn-a39640ebe645b6ba"></a>

## `xv::net::detail::allocate_page`

```doxygen
@fn void *xv::net::detail::allocate_page()
@brief 向当前物理分配器申请一个网络缓冲页。
@return 独占物理页，失败为 nullptr；内存未清零，不得发送有效长度以外的数据。
```

<a id="fn-9ef32a99d26fe40e"></a>

## `xv::net::detail::free_page`

```doxygen
@fn void xv::net::detail::free_page(void *page)
@brief 归还已经不被 DMA 或接收队列持有的网络页。
@param page 独占的完整分配页。
@details 与 allocate_page 配对；接收页可由 E1000 分配后转交同一分配器释放。
```

<a id="fn-7079e964eeda41e2"></a>

## `xv::net::detail::owner`

```doxygen
@fn int xv::net::detail::owner()
@brief 取得当前进程的 PID，作为 UDP 绑定持有者身份。
@pre 进程系统调用上下文调用；收包中断不调用本接口。
```

<a id="fn-99f626cb5d17f655"></a>

## `xv::net::detail::interrupted`

```doxygen
@fn bool xv::net::detail::interrupted()
@brief 检查当前进程是否有终止或可中断阻塞的信号。
@return 应退出接收等待时为 true。
@details 委托 Process::is_killed，同时记录捕获信号中断，使系统调用完成层正确处理 EINTR/SA_RESTART。
```

<a id="fn-d96c58b6fbc16587"></a>

## `xv::net::detail::copy_in`

```doxygen
@fn int xv::net::detail::copy_in(void *destination, unsigned long source, unsigned count)
@brief 从当前进程用户空间复制待发送 UDP 数据。
@param destination 内核目标缓冲区。
@param source 用户源地址。
@param count 字节数。
@return 复制成功为 0，地址验证或复制失败为负值。
```

<a id="fn-073e4679a830e329"></a>

## `xv::net::detail::copy_out`

```doxygen
@fn int xv::net::detail::copy_out(unsigned long destination, const void *source, unsigned count)
@brief 向当前进程用户空间复制接收数据或来源元信息。
@param destination 用户目标地址。
@param source 内核源数据。
@param count 字节数。
@return 成功 0，地址验证失败为负值。
@pre 不能持 UDP 自旋锁执行可能触发用户页处理的复制。
```

<a id="fn-eeff613785c4e3e3"></a>

## `xv::net::detail::prepare_sleep`

```doxygen
@fn void xv::net::detail::prepare_sleep(void *channel)
@brief 在仍持 UDP 条件锁时登记可被信号中断的端口等待。
@param channel 稳定的端口槽地址。
@details 等待桶及进程锁保证通知不会在条件检查和睡眠之间丢失；调用者随后释放条件锁再 sleep。
```

<a id="fn-b5609c0fae83d6c4"></a>

## `xv::net::detail::sleep`

```doxygen
@fn void xv::net::detail::sleep()
@brief 在已登记的等待通道上让出 CPU。
@pre 调用者已经释放 UDP 条件自旋锁。
@details 提前通知已撤销等待时立即返回；返回后仍需重新检查队列与信号。
```

<a id="fn-44622dca17b535c2"></a>

## `xv::net::detail::wake`

```doxygen
@fn void xv::net::detail::wake(void *channel)
@brief 通知等待同一端口槽的所有进程重新检查队列。
@param channel 入队或最终释放的端口槽。
@details 仅表示条件可能变化，不能假定醒来后一定有数据。
```
