# `kernel/fs/userfs/protocol.h`


[用户态文件系统协议与生命周期](../../../../user-filesystems.md)。消息传递对象编号、属性与有界数组，不包含跨地址空间指针；根编号为 1，版本为 1，单次数据载荷最多 512 字节。

<a id="fn-cddffe70e50be03a"></a>

## `xv::userfs::operation_bit`

```doxygen
@fn constexpr unsigned long xv::userfs::operation_bit(Operation operation)
@brief 把公共文件操作转换为能力位，用于注册校验与请求门禁。
@details 通用 FS 插件注册 ABI：Registration 包含 size/version、最长 31 字节类型名、初始根属性、只读标志、操作能力及 1..512 字节载荷；ProviderInfo 描述运行时实例状态。必需读操作集合与可选写操作以位图表达，原消息 ABI 与聚合 Forget 保留。见 docs/user-filesystems.md 与 docs/fs-plugin-authoring.md。
```

- [`xv::userfs::operation_bit`](protocol.h.md#fn-cddffe70e50be03a)
