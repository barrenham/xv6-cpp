# `kernel/net/filter/decoder.h`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::filter::decode`](decoder.h.md#fn-407caf51f07ec782)
- [`xv::net::filter::read16`](decoder.h.md#fn-f421cec3afa2b964)
- [`xv::net::filter::read32`](decoder.h.md#fn-823bfc1e6f49b7ea)

<a id="fn-f421cec3afa2b964"></a>

## `xv::net::filter::read16`

```doxygen
@fn inline unsigned xv::net::filter::read16(const unsigned char *data)
@brief 从借用的报文字节读取网络序十六位无符号值。
```

<a id="fn-823bfc1e6f49b7ea"></a>

## `xv::net::filter::read32`

```doxygen
@fn inline unsigned xv::net::filter::read32(const unsigned char *data)
@brief 从借用的报文字节读取网络序三十二位无符号值。
```

<a id="fn-407caf51f07ec782"></a>

## `xv::net::filter::decode`

```doxygen
@fn inline bool xv::net::filter::decode(const unsigned char *data, unsigned size, unsigned input_interface, unsigned output_interface, Metadata &metadata)
@brief 校验完整 IPv4 与传输层边界，并生成无报文指针的匹配元数据。
@details 仅接收已完成重组的数据报；尊重 IHL、总长及 TCP 数据偏移，校验 UDP 长度。完整报文与基础 IPv4 校验和由 IPv4 层负责。 实现通过 MetadataDecodeFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
```
