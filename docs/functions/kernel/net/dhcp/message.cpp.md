# `kernel/net/dhcp/message.cpp`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::DhcpCodec::encode`](message.cpp.md#fn-deb698e6bd4db06e)
- [`xv::net::DhcpCodec::options`](message.cpp.md#fn-cc3e4c98fe31fac2)
- [`xv::net::DhcpCodec::decode`](message.cpp.md#fn-824eb37f1fb744c3)
- [`xv::net::DhcpCodec::lease`](message.cpp.md#fn-9113b1a20b9723b9)

<a id="fn-deb698e6bd4db06e"></a>

## `xv::net::DhcpCodec::encode`

```doxygen
@fn unsigned xv::net::DhcpCodec::encode(unsigned char *b, unsigned capacity, const DhcpRequest &r)
@brief 在给定缓冲区编码最小三百字节 DHCP/BOOTP 请求。
@details 包含事务 ID、MAC、客户端 ID、所需的地址/服务器选项、参数请求列表和最大消息大小；有效长度不足时不写出。
@param buffer 调用者提供的内核字节区。
@param capacity 至少 300 字节。
@param request 值类型编码参数，地址均为主机字节序。
@return 编码长度 300，容量不足返回零。
```

<a id="fn-cc3e4c98fe31fac2"></a>

## `xv::net::DhcpCodec::options`

```doxygen
@fn bool xv::net::DhcpCodec::options(const unsigned char *data, unsigned length, DhcpReply &r, unsigned &overload, bool main)
@brief 有界解析 DHCP TLV，拒绝截断、重复关键选项与非法长度。
@details 支持 Pad/End，掩码、路由器、DNS、租期、T1/T2 和一次 BOOTP 选项重载；重载区域不能再嵌套重载，未知合法选项跳过。
@param main 是否主选项区域。
@return 区域有合法 End 且格式完整时为 true。
```

<a id="fn-824eb37f1fb744c3"></a>

## `xv::net::DhcpCodec::decode`

```doxygen
@fn bool xv::net::DhcpCodec::decode(const unsigned char *b, unsigned length, DhcpReply &r)
@brief 验证 BOOTP 回复固定首部、magic cookie、服务器标识及消息类型，解析普通和重载选项。
@param buffer UDP 数据部分，不含 UDP/IP 首部。
@param length 实际可访问的字节长度。
@param[out] reply 清零后填入解析结果，含关键选项存在位。
@return 合法 Offer、ACK 或 NAK 返回 true；失败时不得使用结果安装配置。
```

<a id="fn-9113b1a20b9723b9"></a>

## `xv::net::DhcpCodec::lease`

```doxygen
@fn bool xv::net::DhcpCodec::lease(const DhcpReply &r, DhcpLease &result)
@brief 把 ACK 选项转换为已验证租约，检查地址、连续掩码、非零租期与同链路网关。
@details /31 不使用传统广播地址规则；DNS 保存首个地址，T1/T2 的时序合理性由状态机检查并提供默认值。
@param reply 已解码的 ACK 字段。
@param[out] result 主机字节序配置和秒单位租期。
@return 能安装为教学 IPv4 主机配置时返回 true。
```
