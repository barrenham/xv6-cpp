# `kernel/fs/journal/flow.cppm`

流程与资源约定见 [内核流程迁移](../../../../kernel-flows.md).

## 函数导航


<a id="fn-53df8f2cbecf5834"></a>

## `xv::fs::Xv6Journal::WaitProtocol::wait`

```doxygen
@fn static void xv::fs::Xv6Journal::WaitProtocol::wait(Context &c) noexcept
@brief 登记日志等待通道，解锁睡眠后重新取得状态锁。
@details 原xv6日志实现移到内部模块分区；Begin/End/Commit/Recover/Record/Sync流程图、pin与持久化顺序保持。
```

<a id="fn-ac27f877d0d9c219"></a>

## `xv::fs::Xv6Journal::WaitProtocol::unlock`

```doxygen
@fn static void xv::fs::Xv6Journal::WaitProtocol::unlock(Context &c) noexcept
@brief 释放日志状态锁。
@details 原xv6日志实现移到内部模块分区；Begin/End/Commit/Recover/Record/Sync流程图、pin与持久化顺序保持。
```

<a id="fn-403050d70396cbcb"></a>

## `xv::fs::Xv6Journal::BeginFlow::committing`

```doxygen
@fn static bool xv::fs::Xv6Journal::BeginFlow::committing(Context &c) noexcept
@brief 检查日志是否正在提交。
@details 原xv6日志实现移到内部模块分区；Begin/End/Commit/Recover/Record/Sync流程图、pin与持久化顺序保持。
```

<a id="fn-ae0359d07d125cd7"></a>

## `xv::fs::Xv6Journal::BeginFlow::space_needed`

```doxygen
@fn static bool xv::fs::Xv6Journal::BeginFlow::space_needed(Context &c) noexcept
@brief 按现有 outstanding 与最大操作块数检查预留容量。
@details 原xv6日志实现移到内部模块分区；Begin/End/Commit/Recover/Record/Sync流程图、pin与持久化顺序保持。
```

<a id="fn-9d0f7f7cea24ae89"></a>

## `xv::fs::Xv6Journal::BeginFlow::reserve`

```doxygen
@fn static void xv::fs::Xv6Journal::BeginFlow::reserve(Context &c) noexcept
@brief 增加未结束操作数量。
@details 原xv6日志实现移到内部模块分区；Begin/End/Commit/Recover/Record/Sync流程图、pin与持久化顺序保持。
```

<a id="fn-977b30c1c1f2fa0f"></a>

## `xv::fs::Xv6Journal::CommitFlow::dirty`

```doxygen
@fn static bool xv::fs::Xv6Journal::CommitFlow::dirty(Context &c) noexcept
@brief 检查本轮是否存在需要提交的日志块。
@details 原xv6日志实现移到内部模块分区；Begin/End/Commit/Recover/Record/Sync流程图、pin与持久化顺序保持。
```

<a id="fn-9e59ba5bb2a7b967"></a>

## `xv::fs::Xv6Journal::CommitFlow::write_log`

```doxygen
@fn static void xv::fs::Xv6Journal::CommitFlow::write_log(Context &c) noexcept
@brief 写入日志数据并执行持久化 flush。
@details 原xv6日志实现移到内部模块分区；Begin/End/Commit/Recover/Record/Sync流程图、pin与持久化顺序保持。
```

<a id="fn-359f0462d75e6c73"></a>

## `xv::fs::Xv6Journal::CommitFlow::persist_header`

```doxygen
@fn static void xv::fs::Xv6Journal::CommitFlow::persist_header(Context &c) noexcept
@brief 写入当前日志头并执行持久化 flush。
@details 原xv6日志实现移到内部模块分区；Begin/End/Commit/Recover/Record/Sync流程图、pin与持久化顺序保持。
```

<a id="fn-f3afa5f02046f84a"></a>

## `xv::fs::Xv6Journal::CommitFlow::install`

```doxygen
@fn static void xv::fs::Xv6Journal::CommitFlow::install(Context &c) noexcept
@brief 安装已提交块到 home 位置并执行 flush。
@details 原xv6日志实现移到内部模块分区；Begin/End/Commit/Recover/Record/Sync流程图、pin与持久化顺序保持。
```

<a id="fn-40f0fac783421100"></a>

## `xv::fs::Xv6Journal::CommitFlow::clear`

```doxygen
@fn static void xv::fs::Xv6Journal::CommitFlow::clear(Context &c) noexcept
@brief 清零内存日志头的有效块数。
@details 原xv6日志实现移到内部模块分区；Begin/End/Commit/Recover/Record/Sync流程图、pin与持久化顺序保持。
```

<a id="fn-9788719ceaebad0c"></a>

## `xv::fs::Xv6Journal::RecoverFlow::read_header`

```doxygen
@fn static void xv::fs::Xv6Journal::RecoverFlow::read_header(Context &c) noexcept
@brief 读取持久日志头。
@details 原xv6日志实现移到内部模块分区；Begin/End/Commit/Recover/Record/Sync流程图、pin与持久化顺序保持。
```

<a id="fn-f5dd676433ad6840"></a>

## `xv::fs::Xv6Journal::RecoverFlow::install`

```doxygen
@fn static void xv::fs::Xv6Journal::RecoverFlow::install(Context &c) noexcept
@brief 以 recovering 模式重放日志块并 flush。
@details 原xv6日志实现移到内部模块分区；Begin/End/Commit/Recover/Record/Sync流程图、pin与持久化顺序保持。
```

<a id="fn-724c6580e5085080"></a>

## `xv::fs::Xv6Journal::RecoverFlow::clear`

```doxygen
@fn static void xv::fs::Xv6Journal::RecoverFlow::clear(Context &c) noexcept
@brief 清零已恢复日志的内存有效块数。
@details 原xv6日志实现移到内部模块分区；Begin/End/Commit/Recover/Record/Sync流程图、pin与持久化顺序保持。
```

<a id="fn-a2c5096ac72881d8"></a>

## `xv::fs::Xv6Journal::EndFlow::finish_operation`

```doxygen
@fn static void xv::fs::Xv6Journal::EndFlow::finish_operation(Context &c) noexcept
@brief 减少未结束操作数量。
@details 原xv6日志实现移到内部模块分区；Begin/End/Commit/Recover/Record/Sync流程图、pin与持久化顺序保持。
```

<a id="fn-e11f01f6f4895f8a"></a>

## `xv::fs::Xv6Journal::EndFlow::committing`

```doxygen
@fn static bool xv::fs::Xv6Journal::EndFlow::committing(Context &c) noexcept
@brief 检查是否违反提交期间结束操作的协议。
@details 原xv6日志实现移到内部模块分区；Begin/End/Commit/Recover/Record/Sync流程图、pin与持久化顺序保持。
```

<a id="fn-1768f61358a8efa6"></a>

## `xv::fs::Xv6Journal::EndFlow::invalid`

```doxygen
@fn static void xv::fs::Xv6Journal::EndFlow::invalid(Context &) noexcept
@brief 诊断不允许的 committing 状态。
@details 原xv6日志实现移到内部模块分区；Begin/End/Commit/Recover/Record/Sync流程图、pin与持久化顺序保持。
```

<a id="fn-ba21117c75dded39"></a>

## `xv::fs::Xv6Journal::EndFlow::last`

```doxygen
@fn static bool xv::fs::Xv6Journal::EndFlow::last(Context &c) noexcept
@brief 检查是否为最后一个未结束操作。
@details 原xv6日志实现移到内部模块分区；Begin/End/Commit/Recover/Record/Sync流程图、pin与持久化顺序保持。
```

<a id="fn-e3f618a3e92e1202"></a>

## `xv::fs::Xv6Journal::EndFlow::mark_commit`

```doxygen
@fn static void xv::fs::Xv6Journal::EndFlow::mark_commit(Context &c) noexcept
@brief 标记本轮由当前操作负责提交。
@details 原xv6日志实现移到内部模块分区；Begin/End/Commit/Recover/Record/Sync流程图、pin与持久化顺序保持。
```

<a id="fn-d2c1ba1a7994a9da"></a>

## `xv::fs::Xv6Journal::EndFlow::notify`

```doxygen
@fn static void xv::fs::Xv6Journal::EndFlow::notify(Context &c) noexcept
@brief 通知等待日志预留空间的操作。
@details 原xv6日志实现移到内部模块分区；Begin/End/Commit/Recover/Record/Sync流程图、pin与持久化顺序保持。
```

<a id="fn-a2e7f3db6ba0d79c"></a>

## `xv::fs::Xv6Journal::EndFlow::commit`

```doxygen
@fn static void xv::fs::Xv6Journal::EndFlow::commit(Context &c) noexcept
@brief 提交暂存事务，提交记录落盘后再checkpoint并清理日志。
@details 原xv6日志实现移到内部模块分区；Begin/End/Commit/Recover/Record/Sync流程图、pin与持久化顺序保持。
```

<a id="fn-0471cc7476c10e8c"></a>

## `xv::fs::Xv6Journal::EndFlow::finish_commit`

```doxygen
@fn static void xv::fs::Xv6Journal::EndFlow::finish_commit(Context &c) noexcept
@brief 重新加锁清除 committing、增加提交计数并通知后解锁。
@details 原xv6日志实现移到内部模块分区；Begin/End/Commit/Recover/Record/Sync流程图、pin与持久化顺序保持。
```

<a id="fn-3e4230072ef45644"></a>

## `xv::fs::Xv6Journal::RecordFlow::full`

```doxygen
@fn static bool xv::fs::Xv6Journal::RecordFlow::full(Context &c) noexcept
@brief 检查日志头是否已经达到最大块数。
@details 原xv6日志实现移到内部模块分区；Begin/End/Commit/Recover/Record/Sync流程图、pin与持久化顺序保持。
```

<a id="fn-aa1cb338dc5db552"></a>

## `xv::fs::Xv6Journal::RecordFlow::too_big`

```doxygen
@fn static void xv::fs::Xv6Journal::RecordFlow::too_big(Context &) noexcept
@brief 诊断事务过大。
@details 原xv6日志实现移到内部模块分区；Begin/End/Commit/Recover/Record/Sync流程图、pin与持久化顺序保持。
```

<a id="fn-166e9fc57ce14465"></a>

## `xv::fs::Xv6Journal::RecordFlow::active`

```doxygen
@fn static bool xv::fs::Xv6Journal::RecordFlow::active(Context &c) noexcept
@brief 检查是否有已开始的事务。
@details 原xv6日志实现移到内部模块分区；Begin/End/Commit/Recover/Record/Sync流程图、pin与持久化顺序保持。
```

<a id="fn-090240ed44c10ca3"></a>

## `xv::fs::Xv6Journal::RecordFlow::outside`

```doxygen
@fn static void xv::fs::Xv6Journal::RecordFlow::outside(Context &) noexcept
@brief 诊断在事务外记录日志。
@details 原xv6日志实现移到内部模块分区；Begin/End/Commit/Recover/Record/Sync流程图、pin与持久化顺序保持。
```

<a id="fn-25e09a56fa876aea"></a>

## `xv::fs::Xv6Journal::RecordFlow::scan`

```doxygen
@fn static void xv::fs::Xv6Journal::RecordFlow::scan(Context &c) noexcept
@brief 扫描标准JBD记录，核对序号、descriptor、commit和revoke。
@details 原xv6日志实现移到内部模块分区；Begin/End/Commit/Recover/Record/Sync流程图、pin与持久化顺序保持。
```

<a id="fn-255870cc8bf6584b"></a>

## `xv::fs::Xv6Journal::RecordFlow::record`

```doxygen
@fn static void xv::fs::Xv6Journal::RecordFlow::record(Context &c) noexcept
@brief 将当前块号写入选中的头条目。
@details 原xv6日志实现移到内部模块分区；Begin/End/Commit/Recover/Record/Sync流程图、pin与持久化顺序保持。
```

<a id="fn-894a39bf895d8bd8"></a>

## `xv::fs::Xv6Journal::RecordFlow::fresh`

```doxygen
@fn static bool xv::fs::Xv6Journal::RecordFlow::fresh(Context &c) noexcept
@brief 检查本次是否追加新的日志头条目。
@details 原xv6日志实现移到内部模块分区；Begin/End/Commit/Recover/Record/Sync流程图、pin与持久化顺序保持。
```

<a id="fn-8ecf48e18cfcb3a7"></a>

## `xv::fs::Xv6Journal::RecordFlow::pin`

```doxygen
@fn static void xv::fs::Xv6Journal::RecordFlow::pin(Context &c) noexcept
@brief pin 新记录的数据块并增加有效块数。
@details 原xv6日志实现移到内部模块分区；Begin/End/Commit/Recover/Record/Sync流程图、pin与持久化顺序保持。
```

<a id="fn-b731c44216c9927c"></a>

## `xv::fs::Xv6Journal::SyncFlow::active`

```doxygen
@fn static bool xv::fs::Xv6Journal::SyncFlow::active(Context &c) noexcept
@brief 检查是否需要等待正在进行的操作或提交。
@details 原xv6日志实现移到内部模块分区；Begin/End/Commit/Recover/Record/Sync流程图、pin与持久化顺序保持。
```

<a id="fn-c256cec8a4535957"></a>

## `xv::fs::Xv6Journal::SyncFlow::target`

```doxygen
@fn static void xv::fs::Xv6Journal::SyncFlow::target(Context &c) noexcept
@brief 保存当前提交计数加一的等待目标。
@details 原xv6日志实现移到内部模块分区；Begin/End/Commit/Recover/Record/Sync流程图、pin与持久化顺序保持。
```

<a id="fn-1734111a31b651e8"></a>

## `xv::fs::Xv6Journal::SyncFlow::waiting`

```doxygen
@fn static bool xv::fs::Xv6Journal::SyncFlow::waiting(Context &c) noexcept
@brief 检查提交计数是否尚未达到保存的目标。
@details 原xv6日志实现移到内部模块分区；Begin/End/Commit/Recover/Record/Sync流程图、pin与持久化顺序保持。
```

<a id="fn-a7aa29fcf3a6c05a"></a>

## `xv::fs::Xv6Journal::SyncFlow::flush`

```doxygen
@fn static void xv::fs::Xv6Journal::SyncFlow::flush(Context &c) noexcept
@brief 在状态锁外刷新块缓存。
@details 原xv6日志实现移到内部模块分区；Begin/End/Commit/Recover/Record/Sync流程图、pin与持久化顺序保持。
```
