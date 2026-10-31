# `kernel/fs/ext3/format.h`

<a id="fn-3d46ae22fc28fd63"></a>

## `xv::ext3::le16`

```doxygen
@fn inline u16 xv::ext3::le16(const void *memory)
@brief 返回 `u16(p[0])|(u16(p[1])<<8)`。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-b25191749d3451ac"></a>

## `xv::ext3::le32`

```doxygen
@fn inline u32 xv::ext3::le32(const void *memory)
@brief 返回 `u32(p[0])|(u32(p[1])<<8)|(u32(p[2])<<16)|(u32(p[3])<<24)`。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-61e3d74043ad803f"></a>

## `xv::ext3::be32`

```doxygen
@fn inline u32 xv::ext3::be32(const void *memory)
@brief 返回 `(u32(p[0])<<24)|(u32(p[1])<<16)|(u32(p[2])<<8)|p[3]`。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-2ab26b11a6256cf5"></a>

## `xv::ext3::put16`

```doxygen
@fn inline void xv::ext3::put16(void *memory,u16 value)
@brief 实现 xv::ext3::put16 的模块操作，按声明参数读写其封装状态。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-1da1459e57db1b39"></a>

## `xv::ext3::put32`

```doxygen
@fn inline void xv::ext3::put32(void *memory,u32 value)
@brief 实现 xv::ext3::put32 的模块操作，按声明参数读写其封装状态。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-8de2f5bea54f3f0a"></a>

## `xv::ext3::put_be32`

```doxygen
@fn inline void xv::ext3::put_be32(void *memory,u32 value)
@brief 实现 xv::ext3::put_be32 的模块操作，按声明参数读写其封装状态。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-1eb934da6de36461"></a>

## `xv::ext3::copy`

```doxygen
@fn inline void xv::ext3::copy(void *destination,const void *source,unsigned count)
@brief 将指定快照字段逐页写入sink，失败立即停止。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-5da6fd7b138a8fea"></a>

## `xv::ext3::zero`

```doxygen
@fn inline void xv::ext3::zero(void *memory,unsigned count)
@brief 实现 xv::ext3::zero 的模块操作，按声明参数读写其封装状态。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-f32ef8f1326aad34"></a>

## `xv::ext3::equal`

```doxygen
@fn inline bool xv::ext3::equal(const void *first,const void *second,unsigned count)
@brief 实现 xv::ext3::equal 的模块操作，按声明参数读写其封装状态。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-775cda481f588328"></a>

## `xv::ext3::sequence_at_least`

```doxygen
@fn inline bool xv::ext3::sequence_at_least(u32 value,u32 previous)
@brief 返回 `!((value-previous)&0x80000000U)`。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```
