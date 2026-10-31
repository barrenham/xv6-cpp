# `kernel/fs/journal/xv6_journal.cppm`

<a id="fn-cfb29c55fb2dc5d2"></a>

## `xv::fs::Xv6Journal::Xv6Journal`

```doxygen
@fn constexpr explicit xv::fs::Xv6Journal::Xv6Journal(BlockCache &cache) : cache_(cache)
@brief 构造对象并建立成员初值及借用关系。
@details 原xv6日志实现移到内部模块分区；Begin/End/Commit/Recover/Record/Sync流程图、pin与持久化顺序保持。
```

<a id="fn-2a2ef6986f5ab22d"></a>

## `xv::fs::Xv6Journal::Xv6Journal`

```doxygen
@fn xv::fs::Xv6Journal::Xv6Journal(const Xv6Journal &) = delete
@brief 规定对象赋值/复制约束，防止资源所有权被隐式复制。
@details 原xv6日志实现移到内部模块分区；Begin/End/Commit/Recover/Record/Sync流程图、pin与持久化顺序保持。
```

<a id="fn-7a0dccffa71496be"></a>

## `xv::fs::Xv6Journal::operator=`

```doxygen
@fn Xv6Journal &xv::fs::Xv6Journal::operator=(const Xv6Journal &) = delete
@brief 声明与具体磁盘格式无关的事务协议；由xv6或ext3后端实现。
@details 原xv6日志实现移到内部模块分区；Begin/End/Commit/Recover/Record/Sync流程图、pin与持久化顺序保持。
```
