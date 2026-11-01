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

- [`xv::fs::Xv6Journal::Xv6Journal`](xv6_journal.cppm.md#fn-cfb29c55fb2dc5d2)

- [`xv::fs::Xv6Journal::Xv6Journal`](xv6_journal.cppm.md#fn-2a2ef6986f5ab22d)

- [`xv::fs::Xv6Journal::begin`](journal.cpp.md#fn-3b99e251cc23f313)

- [`xv::fs::Xv6Journal::commit`](journal.cpp.md#fn-bb993364d2886f1f)

- [`xv::fs::Xv6Journal::end`](journal.cpp.md#fn-6d7aaae2b60d81c0)

- [`xv::fs::Xv6Journal::initialize`](journal.cpp.md#fn-2c20a16c196f2788)

- [`xv::fs::Xv6Journal::install`](journal.cpp.md#fn-3fbd7fa4976a0fae)

- [`xv::fs::Xv6Journal::operator=`](xv6_journal.cppm.md#fn-7a0dccffa71496be)

- [`xv::fs::Xv6Journal::read_header`](journal.cpp.md#fn-7fcbaae63541144f)

- [`xv::fs::Xv6Journal::record`](journal.cpp.md#fn-33a9575eb4f8ab1c)

- [`xv::fs::Xv6Journal::recover`](journal.cpp.md#fn-8d622b993f20da3b)

- [`xv::fs::Xv6Journal::sync`](journal.cpp.md#fn-acadc171bcf1eb5f)

- [`xv::fs::Xv6Journal::write_header`](journal.cpp.md#fn-93985a1f7243908c)

- [`xv::fs::Xv6Journal::write_log`](journal.cpp.md#fn-9fd9c9d77fa55de6)
