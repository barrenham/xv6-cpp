# `kernel/fs/journal/flow.cppm`

流程与资源约定见 [内核流程迁移](../../../../kernel-flows.md).

## 函数导航

- [`xv::fs::Journal::BeginFlow::committing`](flow.cppm.md#fn-fd066df5eb76d91e)
- [`xv::fs::Journal::BeginFlow::reserve`](flow.cppm.md#fn-767a634898524f56)
- [`xv::fs::Journal::BeginFlow::space_needed`](flow.cppm.md#fn-2cf4fd839d5c9a94)
- [`xv::fs::Journal::CommitFlow::clear`](flow.cppm.md#fn-e097404fcf3ce5a9)
- [`xv::fs::Journal::CommitFlow::dirty`](flow.cppm.md#fn-6a08a129aede3703)
- [`xv::fs::Journal::CommitFlow::install`](flow.cppm.md#fn-cb6150a23a136ae1)
- [`xv::fs::Journal::CommitFlow::persist_header`](flow.cppm.md#fn-66a327610e8573e9)
- [`xv::fs::Journal::CommitFlow::write_log`](flow.cppm.md#fn-43a6acda71245495)
- [`xv::fs::Journal::EndFlow::commit`](flow.cppm.md#fn-7cb93f3256a677f0)
- [`xv::fs::Journal::EndFlow::committing`](flow.cppm.md#fn-4600609014c6d81e)
- [`xv::fs::Journal::EndFlow::finish_commit`](flow.cppm.md#fn-8ecac2119dfaec6b)
- [`xv::fs::Journal::EndFlow::finish_operation`](flow.cppm.md#fn-a0aa6cb6a0d3b66a)
- [`xv::fs::Journal::EndFlow::invalid`](flow.cppm.md#fn-24918c26702bbb1b)
- [`xv::fs::Journal::EndFlow::last`](flow.cppm.md#fn-ffaa945cb2d70c85)
- [`xv::fs::Journal::EndFlow::mark_commit`](flow.cppm.md#fn-1d458c8139d18459)
- [`xv::fs::Journal::EndFlow::notify`](flow.cppm.md#fn-c495ed75abb02d49)
- [`xv::fs::Journal::RecordFlow::active`](flow.cppm.md#fn-5c5485e1d5b0c3a6)
- [`xv::fs::Journal::RecordFlow::fresh`](flow.cppm.md#fn-b5431575633410e2)
- [`xv::fs::Journal::RecordFlow::full`](flow.cppm.md#fn-3701527042521853)
- [`xv::fs::Journal::RecordFlow::outside`](flow.cppm.md#fn-51bdf45d790e3e65)
- [`xv::fs::Journal::RecordFlow::pin`](flow.cppm.md#fn-fc5cec0ad58668ab)
- [`xv::fs::Journal::RecordFlow::record`](flow.cppm.md#fn-adad0beb11bea962)
- [`xv::fs::Journal::RecordFlow::scan`](flow.cppm.md#fn-502bf8533e9504e9)
- [`xv::fs::Journal::RecordFlow::too_big`](flow.cppm.md#fn-09f5403311f29d2f)
- [`xv::fs::Journal::RecoverFlow::clear`](flow.cppm.md#fn-0b2782134c3086bc)
- [`xv::fs::Journal::RecoverFlow::install`](flow.cppm.md#fn-55b37e6ab4c65dbc)
- [`xv::fs::Journal::RecoverFlow::read_header`](flow.cppm.md#fn-82facc1d3cddc293)
- [`xv::fs::Journal::SyncFlow::active`](flow.cppm.md#fn-9a346b1f743b50d7)
- [`xv::fs::Journal::SyncFlow::flush`](flow.cppm.md#fn-0374406d135a4a25)
- [`xv::fs::Journal::SyncFlow::target`](flow.cppm.md#fn-3012b9e136ff6cfd)
- [`xv::fs::Journal::SyncFlow::waiting`](flow.cppm.md#fn-e8108d2190139155)
- [`xv::fs::Journal::WaitProtocol::unlock`](flow.cppm.md#fn-8879ee5b1575347e)
- [`xv::fs::Journal::WaitProtocol::wait`](flow.cppm.md#fn-5d20fcd1684418e3)

<a id="fn-fd066df5eb76d91e"></a>

## `xv::fs::Journal::BeginFlow::committing`

```doxygen
@fn static bool xv::fs::Journal::BeginFlow::committing(Context &c) noexcept
@brief 检查日志是否正在提交。
@details 入口已持日志状态锁；正在提交与容量不足共享相同等待动作，醒来重新检查两种条件。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-767a634898524f56"></a>

## `xv::fs::Journal::BeginFlow::reserve`

```doxygen
@fn static void xv::fs::Journal::BeginFlow::reserve(Context &c) noexcept
@brief 增加未结束操作数量。
@details 入口已持日志状态锁；正在提交与容量不足共享相同等待动作，醒来重新检查两种条件。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-2cf4fd839d5c9a94"></a>

## `xv::fs::Journal::BeginFlow::space_needed`

```doxygen
@fn static bool xv::fs::Journal::BeginFlow::space_needed(Context &c) noexcept
@brief 按现有 outstanding 与最大操作块数检查预留容量。
@details 入口已持日志状态锁；正在提交与容量不足共享相同等待动作，醒来重新检查两种条件。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-e097404fcf3ce5a9"></a>

## `xv::fs::Journal::CommitFlow::clear`

```doxygen
@fn static void xv::fs::Journal::CommitFlow::clear(Context &c) noexcept
@brief 清零内存日志头的有效块数。
@details 提交图在独占提交阶段执行 I/O，不持日志自旋锁；persist_header 也供恢复图使用，两个头节点显式重复执行同一动作。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-6a08a129aede3703"></a>

## `xv::fs::Journal::CommitFlow::dirty`

```doxygen
@fn static bool xv::fs::Journal::CommitFlow::dirty(Context &c) noexcept
@brief 检查本轮是否存在需要提交的日志块。
@details 提交图在独占提交阶段执行 I/O，不持日志自旋锁；persist_header 也供恢复图使用，两个头节点显式重复执行同一动作。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-cb6150a23a136ae1"></a>

## `xv::fs::Journal::CommitFlow::install`

```doxygen
@fn static void xv::fs::Journal::CommitFlow::install(Context &c) noexcept
@brief 安装已提交块到 home 位置并执行 flush。
@details 提交图在独占提交阶段执行 I/O，不持日志自旋锁；persist_header 也供恢复图使用，两个头节点显式重复执行同一动作。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-66a327610e8573e9"></a>

## `xv::fs::Journal::CommitFlow::persist_header`

```doxygen
@fn static void xv::fs::Journal::CommitFlow::persist_header(Context &c) noexcept
@brief 写入当前日志头并执行持久化 flush。
@details 提交图在独占提交阶段执行 I/O，不持日志自旋锁；persist_header 也供恢复图使用，两个头节点显式重复执行同一动作。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-43a6acda71245495"></a>

## `xv::fs::Journal::CommitFlow::write_log`

```doxygen
@fn static void xv::fs::Journal::CommitFlow::write_log(Context &c) noexcept
@brief 写入日志数据并执行持久化 flush。
@details 提交图在独占提交阶段执行 I/O，不持日志自旋锁；persist_header 也供恢复图使用，两个头节点显式重复执行同一动作。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-7cb93f3256a677f0"></a>

## `xv::fs::Journal::EndFlow::commit`

```doxygen
@fn static void xv::fs::Journal::EndFlow::commit(Context &c) noexcept
@brief 在日志自旋锁外执行提交方法。
@details 入口已持日志锁；最后一个操作标记 committing 后解锁提交，完成后重新持锁发布计数并通知等待者。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-4600609014c6d81e"></a>

## `xv::fs::Journal::EndFlow::committing`

```doxygen
@fn static bool xv::fs::Journal::EndFlow::committing(Context &c) noexcept
@brief 检查是否违反提交期间结束操作的协议。
@details 入口已持日志锁；最后一个操作标记 committing 后解锁提交，完成后重新持锁发布计数并通知等待者。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-8ecac2119dfaec6b"></a>

## `xv::fs::Journal::EndFlow::finish_commit`

```doxygen
@fn static void xv::fs::Journal::EndFlow::finish_commit(Context &c) noexcept
@brief 重新加锁清除 committing、增加提交计数并通知后解锁。
@details 入口已持日志锁；最后一个操作标记 committing 后解锁提交，完成后重新持锁发布计数并通知等待者。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-a0aa6cb6a0d3b66a"></a>

## `xv::fs::Journal::EndFlow::finish_operation`

```doxygen
@fn static void xv::fs::Journal::EndFlow::finish_operation(Context &c) noexcept
@brief 减少未结束操作数量。
@details 入口已持日志锁；最后一个操作标记 committing 后解锁提交，完成后重新持锁发布计数并通知等待者。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-24918c26702bbb1b"></a>

## `xv::fs::Journal::EndFlow::invalid`

```doxygen
@fn static void xv::fs::Journal::EndFlow::invalid(Context &) noexcept
@brief 诊断不允许的 committing 状态。
@details 入口已持日志锁；最后一个操作标记 committing 后解锁提交，完成后重新持锁发布计数并通知等待者。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-ffaa945cb2d70c85"></a>

## `xv::fs::Journal::EndFlow::last`

```doxygen
@fn static bool xv::fs::Journal::EndFlow::last(Context &c) noexcept
@brief 检查是否为最后一个未结束操作。
@details 入口已持日志锁；最后一个操作标记 committing 后解锁提交，完成后重新持锁发布计数并通知等待者。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-1d458c8139d18459"></a>

## `xv::fs::Journal::EndFlow::mark_commit`

```doxygen
@fn static void xv::fs::Journal::EndFlow::mark_commit(Context &c) noexcept
@brief 标记本轮由当前操作负责提交。
@details 入口已持日志锁；最后一个操作标记 committing 后解锁提交，完成后重新持锁发布计数并通知等待者。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-c495ed75abb02d49"></a>

## `xv::fs::Journal::EndFlow::notify`

```doxygen
@fn static void xv::fs::Journal::EndFlow::notify(Context &c) noexcept
@brief 通知等待日志预留空间的操作。
@details 入口已持日志锁；最后一个操作标记 committing 后解锁提交，完成后重新持锁发布计数并通知等待者。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-5c5485e1d5b0c3a6"></a>

## `xv::fs::Journal::RecordFlow::active`

```doxygen
@fn static bool xv::fs::Journal::RecordFlow::active(Context &c) noexcept
@brief 检查是否有已开始的事务。
@details 入口持日志锁；容量与事务检查先于吸收扫描，重复记录不重复 pin，新块才增加有效块数。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-b5431575633410e2"></a>

## `xv::fs::Journal::RecordFlow::fresh`

```doxygen
@fn static bool xv::fs::Journal::RecordFlow::fresh(Context &c) noexcept
@brief 检查本次是否追加新的日志头条目。
@details 入口持日志锁；容量与事务检查先于吸收扫描，重复记录不重复 pin，新块才增加有效块数。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-3701527042521853"></a>

## `xv::fs::Journal::RecordFlow::full`

```doxygen
@fn static bool xv::fs::Journal::RecordFlow::full(Context &c) noexcept
@brief 检查日志头是否已经达到最大块数。
@details 入口持日志锁；容量与事务检查先于吸收扫描，重复记录不重复 pin，新块才增加有效块数。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-51bdf45d790e3e65"></a>

## `xv::fs::Journal::RecordFlow::outside`

```doxygen
@fn static void xv::fs::Journal::RecordFlow::outside(Context &) noexcept
@brief 诊断在事务外记录日志。
@details 入口持日志锁；容量与事务检查先于吸收扫描，重复记录不重复 pin，新块才增加有效块数。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-fc5cec0ad58668ab"></a>

## `xv::fs::Journal::RecordFlow::pin`

```doxygen
@fn static void xv::fs::Journal::RecordFlow::pin(Context &c) noexcept
@brief pin 新记录的数据块并增加有效块数。
@details 入口持日志锁；容量与事务检查先于吸收扫描，重复记录不重复 pin，新块才增加有效块数。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-adad0beb11bea962"></a>

## `xv::fs::Journal::RecordFlow::record`

```doxygen
@fn static void xv::fs::Journal::RecordFlow::record(Context &c) noexcept
@brief 将当前块号写入选中的头条目。
@details 入口持日志锁；容量与事务检查先于吸收扫描，重复记录不重复 pin，新块才增加有效块数。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-502bf8533e9504e9"></a>

## `xv::fs::Journal::RecordFlow::scan`

```doxygen
@fn static void xv::fs::Journal::RecordFlow::scan(Context &c) noexcept
@brief 直接扫描当前块号是否已在日志头中。
@details 入口持日志锁；容量与事务检查先于吸收扫描，重复记录不重复 pin，新块才增加有效块数。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-09f5403311f29d2f"></a>

## `xv::fs::Journal::RecordFlow::too_big`

```doxygen
@fn static void xv::fs::Journal::RecordFlow::too_big(Context &) noexcept
@brief 诊断事务过大。
@details 入口持日志锁；容量与事务检查先于吸收扫描，重复记录不重复 pin，新块才增加有效块数。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-0b2782134c3086bc"></a>

## `xv::fs::Journal::RecoverFlow::clear`

```doxygen
@fn static void xv::fs::Journal::RecoverFlow::clear(Context &c) noexcept
@brief 清零已恢复日志的内存有效块数。
@details 在文件系统初始化时执行；先重放已提交数据并 flush，再清除持久日志头，与正常提交共用持久头写入动作。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-55b37e6ab4c65dbc"></a>

## `xv::fs::Journal::RecoverFlow::install`

```doxygen
@fn static void xv::fs::Journal::RecoverFlow::install(Context &c) noexcept
@brief 以 recovering 模式重放日志块并 flush。
@details 在文件系统初始化时执行；先重放已提交数据并 flush，再清除持久日志头，与正常提交共用持久头写入动作。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-82facc1d3cddc293"></a>

## `xv::fs::Journal::RecoverFlow::read_header`

```doxygen
@fn static void xv::fs::Journal::RecoverFlow::read_header(Context &c) noexcept
@brief 读取持久日志头。
@details 在文件系统初始化时执行；先重放已提交数据并 flush，再清除持久日志头，与正常提交共用持久头写入动作。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-9a346b1f743b50d7"></a>

## `xv::fs::Journal::SyncFlow::active`

```doxygen
@fn static bool xv::fs::Journal::SyncFlow::active(Context &c) noexcept
@brief 检查是否需要等待正在进行的操作或提交。
@details 入口持日志状态锁；有活跃操作或提交时等待下一次 commits，结束等待后先解锁再刷新缓存。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-0374406d135a4a25"></a>

## `xv::fs::Journal::SyncFlow::flush`

```doxygen
@fn static void xv::fs::Journal::SyncFlow::flush(Context &c) noexcept
@brief 在状态锁外刷新块缓存。
@details 入口持日志状态锁；有活跃操作或提交时等待下一次 commits，结束等待后先解锁再刷新缓存。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-3012b9e136ff6cfd"></a>

## `xv::fs::Journal::SyncFlow::target`

```doxygen
@fn static void xv::fs::Journal::SyncFlow::target(Context &c) noexcept
@brief 保存当前提交计数加一的等待目标。
@details 入口持日志状态锁；有活跃操作或提交时等待下一次 commits，结束等待后先解锁再刷新缓存。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-e8108d2190139155"></a>

## `xv::fs::Journal::SyncFlow::waiting`

```doxygen
@fn static bool xv::fs::Journal::SyncFlow::waiting(Context &c) noexcept
@brief 检查提交计数是否尚未达到保存的目标。
@details 入口持日志状态锁；有活跃操作或提交时等待下一次 commits，结束等待后先解锁再刷新缓存。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-8879ee5b1575347e"></a>

## `xv::fs::Journal::WaitProtocol::unlock`

```doxygen
@fn static void xv::fs::Journal::WaitProtocol::unlock(Context &c) noexcept
@brief 释放日志状态锁。
@details 调用者持有日志状态锁；wait 在锁下登记，解锁后睡眠并重新持锁返回，unlock 只释放现有锁。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-5d20fcd1684418e3"></a>

## `xv::fs::Journal::WaitProtocol::wait`

```doxygen
@fn static void xv::fs::Journal::WaitProtocol::wait(Context &c) noexcept
@brief 登记日志等待通道，解锁睡眠后重新取得状态锁。
@details 调用者持有日志状态锁；wait 在锁下登记，解锁后睡眠并重新持锁返回，unlock 只释放现有锁。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```
