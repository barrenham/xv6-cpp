# `kernel/net/core/wire.cppm`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::wire::before`](wire.cppm.md#fn-04826648abdb46e6)
- [`xv::net::wire::copy`](wire.cppm.md#fn-f404a3b1ccf22d84)
- [`xv::net::wire::equal`](wire.cppm.md#fn-fe315e7951ac8144)
- [`xv::net::wire::read16`](wire.cppm.md#fn-03097c10e32cc1a1)
- [`xv::net::wire::read32`](wire.cppm.md#fn-5b8eedf076970009)
- [`xv::net::wire::sum`](wire.cppm.md#fn-a3807f23f4c0d7fd)
- [`xv::net::wire::write16`](wire.cppm.md#fn-1bc88c4d8628e598)
- [`xv::net::wire::write32`](wire.cppm.md#fn-13e75d50c52e6be9)
- [`xv::net::wire::zero`](wire.cppm.md#fn-45e0ae947319ccd7)

<a id="fn-03097c10e32cc1a1"></a>

## `xv::net::wire::read16`

```doxygen
@fn inline unsigned xv::net::wire::read16(const unsigned char *p)
@brief 协议字节工具：从至少两字节网络序缓冲区读取主机序数值。
@details 从至少两字节网络序缓冲区读取主机序数值，不要求对齐。
```

<a id="fn-5b8eedf076970009"></a>

## `xv::net::wire::read32`

```doxygen
@fn inline unsigned xv::net::wire::read32(const unsigned char *p)
@brief 协议字节工具：从至少四字节网络序缓冲区组合主机序 IPv4/序号数值。。
@details 从至少四字节网络序缓冲区组合主机序 IPv4/序号数值。
```

<a id="fn-1bc88c4d8628e598"></a>

## `xv::net::wire::write16`

```doxygen
@fn inline void xv::net::wire::write16(unsigned char *p, unsigned n)
@brief 协议字节工具：把低十六位按网络字节序写入至少两字节目标区域。。
@details 把低十六位按网络字节序写入至少两字节目标区域。
```

<a id="fn-13e75d50c52e6be9"></a>

## `xv::net::wire::write32`

```doxygen
@fn inline void xv::net::wire::write32(unsigned char *p, unsigned n)
@brief 协议字节工具：把 32 位数值按网络字节序写入至少四字节目标区域。。
@details 把 32 位数值按网络字节序写入至少四字节目标区域。
```

<a id="fn-f404a3b1ccf22d84"></a>

## `xv::net::wire::copy`

```doxygen
@fn inline void xv::net::wire::copy(void *to, const void *from, unsigned n)
@brief 协议字节工具：按字节复制 n 个有效字节。
@details 按字节复制 n 个有效字节，源目标应不重叠，不建立新的页面所有权。
```

<a id="fn-45e0ae947319ccd7"></a>

## `xv::net::wire::zero`

```doxygen
@fn inline void xv::net::wire::zero(void *to, unsigned n)
@brief 协议字节工具：把 n 个有效目标字节置零。
@details 把 n 个有效目标字节置零，不访问缓冲范围之外。
```

<a id="fn-fe315e7951ac8144"></a>

## `xv::net::wire::equal`

```doxygen
@fn inline bool xv::net::wire::equal(const void *left, const void *right, unsigned n)
@brief 协议字节工具：比较 n 个有效字节是否完全一致。
@details 比较 n 个有效字节是否完全一致，不解释为带终止符字符串。
```

<a id="fn-a3807f23f4c0d7fd"></a>

## `xv::net::wire::sum`

```doxygen
@fn inline unsigned xv::net::wire::sum(const unsigned char *data, unsigned length, unsigned value = 0)
@brief 协议字节工具：计算并折叠 Internet 反码和。
@details 计算并折叠 Internet 反码和，支持伪首部初始和与奇数尾字节；结果尚未取反，合法含校验字段的和为0xffff。
```

<a id="fn-04826648abdb46e6"></a>

## `xv::net::wire::before`

```doxygen
@fn inline bool xv::net::wire::before(unsigned left, unsigned right)
@brief 协议字节工具：按带符号的 32 位序号差实现 TCP 回绕比较。
@details 按带符号的 32 位序号差实现 TCP 回绕比较，调用者保证比较范围小于半个序号空间。
```
