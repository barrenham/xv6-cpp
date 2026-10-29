# `kernel/net/ip/reassembly.cpp`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::Ipv4Reassembly::accept`](reassembly.cpp.md#fn-5c231e529ccb0714)
- [`xv::net::Ipv4Reassembly::clear`](reassembly.cpp.md#fn-4b1b99130edc04c3)
- [`xv::net::Ipv4Reassembly::find_or_create`](reassembly.cpp.md#fn-8e1325965825a993)
- [`xv::net::Ipv4Reassembly::initialize`](reassembly.cpp.md#fn-2c790a6a9a38aa96)
- [`xv::net::Ipv4Reassembly::insert`](reassembly.cpp.md#fn-bad135e7b04f07ab)
- [`xv::net::Ipv4Reassembly::tick`](reassembly.cpp.md#fn-aa40441828278e4f)

<a id="fn-2c790a6a9a38aa96"></a>

## `xv::net::Ipv4Reassembly::initialize`

```doxygen
@fn void xv::net::Ipv4Reassembly::initialize()
@brief 在发布组件前初始化重组专用锁；不清空已有片段状态。
@pre 服务初始化一次后才接收片段。
```

<a id="fn-8e1325965825a993"></a>

## `xv::net::Ipv4Reassembly::find_or_create`

```doxygen
@fn Ipv4Reassembly::Assembly *xv::net::Ipv4Reassembly::find_or_create(const Key &key, const Packet &fragment, unsigned long now)
@brief 按入口对象身份、IPv4 四元键和报文上下文查找重组槽，没有匹配时在空槽申请页面并记录首部及期限。
@pre 调用者已持重组锁，片段基础首部已验证。
@return 匹配或创建的槽位，四槽满或页面分配失败返回空。
```

<a id="fn-bad135e7b04f07ab"></a>

## `xv::net::Ipv4Reassembly::insert`

```doxygen
@fn bool xv::net::Ipv4Reassembly::insert(Assembly &assembly, const unsigned char *payload, unsigned offset, unsigned length, bool last)
@brief 检查片段是否与已有字节重叠或违反最终长度，再写入载荷、位图与累计长度。
@pre 调用者持重组锁，offset 和 length 已处于 MaxPayload 范围。
@return 插入成功 true，重叠、末片长度冲突或已有数据越过末尾返回 false，由外层整槽丢弃。
```

<a id="fn-5c231e529ccb0714"></a>

## `xv::net::Ipv4Reassembly::accept`

```doxygen
@fn Packet xv::net::Ipv4Reassembly::accept(const void *ingress, const Packet &fragment, unsigned header_length, unsigned total_length, unsigned long now)
@brief 借用已验证片段并更新有界重组槽，完成时返回独占页面的完整数据报。
@details 只支持二十字节基础首部、最多四槽和 4062 字节载荷；非末片长度需八字节对齐。入口与上下文不同的片段不合并；重叠或长度冲突丢弃整槽。完成或丢弃先在锁内摘除槽，再在锁外修复首部或释放页面。 实现通过 AcceptFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
@param ingress 稳定的入口对象身份，仅按地址比较，组件不解引用或依赖接口类。
@param now 本次操作的单调毫秒时刻，新槽十五秒后到期。
@return 完成返回有效 Packet，其余状态返回空；原片段的拥有权始终保留在调用方。
@pre IPv4 版本、校验和及源/目的策略由上层验证。
```

<a id="fn-aa40441828278e4f"></a>

## `xv::net::Ipv4Reassembly::tick`

```doxygen
@fn void xv::net::Ipv4Reassembly::tick(unsigned long now)
@brief 逐槽在锁内摘除到期页面，再在锁外归还物理页。
@details 不分派协议、不调用驱动，保留未到期状态。
```

<a id="fn-4b1b99130edc04c3"></a>

## `xv::net::Ipv4Reassembly::clear`

```doxygen
@fn void xv::net::Ipv4Reassembly::clear()
@brief 释放所有未完成重组页面并恢复空槽，供短期组件显式结束生命周期。
@pre 调用方已停止接收新片段；组件按内核长期服务模型使用，不注册全局自动析构。
```
