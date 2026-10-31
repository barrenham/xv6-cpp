# `kernel/fs/journal/journal.cpp`

开发职责：文件描述、inode、路径、块缓存与日志事务。

源码：[打开实际文件](../../../../../kernel/fs/journal/journal.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.fs`。

直接模块依赖：`:platform`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::fs::Journal::begin`](journal.cpp.md#fn-68275ab1a63eaf6b)
- [`xv::fs::Journal::end`](journal.cpp.md#fn-9a8ba69510b2c5fb)
- [`xv::fs::Journal::record`](journal.cpp.md#fn-2bff0beebb0a1b13)
- [`xv::fs::Journal::sync`](journal.cpp.md#fn-5798daa238e10eb6)

<a id="fn-68275ab1a63eaf6b"></a>

## `xv::fs::Journal::begin`

```doxygen
@fn virtual void xv::fs::Journal::begin()=0
@brief 声明与具体磁盘格式无关的事务协议；由xv6或ext3后端实现。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```

<a id="fn-9a8ba69510b2c5fb"></a>

## `xv::fs::Journal::end`

```doxygen
@fn virtual void xv::fs::Journal::end()=0
@brief 声明与具体磁盘格式无关的事务协议；由xv6或ext3后端实现。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```

<a id="fn-2bff0beebb0a1b13"></a>

## `xv::fs::Journal::record`

```doxygen
@fn virtual void xv::fs::Journal::record(Buffer &buffer)=0
@brief 声明与具体磁盘格式无关的事务协议；由xv6或ext3后端实现。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```

<a id="fn-5798daa238e10eb6"></a>

## `xv::fs::Journal::sync`

```doxygen
@fn virtual void xv::fs::Journal::sync()=0
@brief 声明与具体磁盘格式无关的事务协议；由xv6或ext3后端实现。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```

<a id="fn-2c20a16c196f2788"></a>

## `xv::fs::Xv6Journal::initialize`

```doxygen
@fn void Xv6Journal::xv::fs::Xv6Journal::initialize(int dev, const superblock *sb)
@brief 文件系统重做日志：建立该对象开始工作所需的初始状态。
@details 原xv6日志实现移到内部模块分区；Begin/End/Commit/Recover/Record/Sync流程图、pin与持久化顺序保持。
```

<a id="fn-3fbd7fa4976a0fae"></a>

## `xv::fs::Xv6Journal::install`

```doxygen
@fn void Xv6Journal::xv::fs::Xv6Journal::install(int recovering)
@brief 文件系统重做日志：把已提交日志安装到原始数据块位置。
@details 原xv6日志实现移到内部模块分区；Begin/End/Commit/Recover/Record/Sync流程图、pin与持久化顺序保持。
```

<a id="fn-7fcbaae63541144f"></a>

## `xv::fs::Xv6Journal::read_header`

```doxygen
@fn void Xv6Journal::xv::fs::Xv6Journal::read_header(void)
@brief 文件系统重做日志：从磁盘恢复日志头记录。
@details 原xv6日志实现移到内部模块分区；Begin/End/Commit/Recover/Record/Sync流程图、pin与持久化顺序保持。
```

<a id="fn-93985a1f7243908c"></a>

## `xv::fs::Xv6Journal::write_header`

```doxygen
@fn void Xv6Journal::xv::fs::Xv6Journal::write_header(void)
@brief 文件系统重做日志：写入作为提交点的磁盘日志头。
@details 原xv6日志实现移到内部模块分区；Begin/End/Commit/Recover/Record/Sync流程图、pin与持久化顺序保持。
```

<a id="fn-8d622b993f20da3b"></a>

## `xv::fs::Xv6Journal::recover`

```doxygen
@fn void Xv6Journal::xv::fs::Xv6Journal::recover(void)
@brief 先确认提交和revoke集合，再重放有效事务，最后清理日志头。
@details 原xv6日志实现移到内部模块分区；Begin/End/Commit/Recover/Record/Sync流程图、pin与持久化顺序保持。
```

<a id="fn-3b99e251cc23f313"></a>

## `xv::fs::Xv6Journal::begin`

```doxygen
@fn void Xv6Journal::xv::fs::Xv6Journal::begin(void)
@brief 声明与具体磁盘格式无关的事务协议；由xv6或ext3后端实现。
@details 原xv6日志实现移到内部模块分区；Begin/End/Commit/Recover/Record/Sync流程图、pin与持久化顺序保持。
```

<a id="fn-6d7aaae2b60d81c0"></a>

## `xv::fs::Xv6Journal::end`

```doxygen
@fn void Xv6Journal::xv::fs::Xv6Journal::end(void)
@brief 声明与具体磁盘格式无关的事务协议；由xv6或ext3后端实现。
@details 原xv6日志实现移到内部模块分区；Begin/End/Commit/Recover/Record/Sync流程图、pin与持久化顺序保持。
```

<a id="fn-9fd9c9d77fa55de6"></a>

## `xv::fs::Xv6Journal::write_log`

```doxygen
@fn void Xv6Journal::xv::fs::Xv6Journal::write_log(void)
@brief 文件系统重做日志：把被修改的缓存块复制到日志区。
@details 原xv6日志实现移到内部模块分区；Begin/End/Commit/Recover/Record/Sync流程图、pin与持久化顺序保持。
```

<a id="fn-bb993364d2886f1f"></a>

## `xv::fs::Xv6Journal::commit`

```doxygen
@fn void Xv6Journal::xv::fs::Xv6Journal::commit()
@brief 提交暂存事务，提交记录落盘后再checkpoint并清理日志。
@details 原xv6日志实现移到内部模块分区；Begin/End/Commit/Recover/Record/Sync流程图、pin与持久化顺序保持。
```

<a id="fn-33a9575eb4f8ab1c"></a>

## `xv::fs::Xv6Journal::record`

```doxygen
@fn void Xv6Journal::xv::fs::Xv6Journal::record(Buffer &buffer)
@brief 声明与具体磁盘格式无关的事务协议；由xv6或ext3后端实现。
@details 原xv6日志实现移到内部模块分区；Begin/End/Commit/Recover/Record/Sync流程图、pin与持久化顺序保持。
```

<a id="fn-acadc171bcf1eb5f"></a>

## `xv::fs::Xv6Journal::sync`

```doxygen
@fn void Xv6Journal::xv::fs::Xv6Journal::sync(void)
@brief 声明与具体磁盘格式无关的事务协议；由xv6或ext3后端实现。
@details 原xv6日志实现移到内部模块分区；Begin/End/Commit/Recover/Record/Sync流程图、pin与持久化顺序保持。
```
