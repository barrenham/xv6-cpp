# `kernel/net/filter/filter.cpp`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::Firewall::accept`](filter.cpp.md#fn-b1ddf21079255883)
- [`xv::net::Firewall::execute`](filter.cpp.md#fn-6c6726c5c280a782)
- [`xv::net::Firewall::initialize`](filter.cpp.md#fn-e044e49987bca493)

<a id="fn-e044e49987bca493"></a>

## `xv::net::Firewall::initialize`

```doxygen
@fn void xv::net::Firewall::initialize()
@brief 初始化防火墙服务专用自旋锁；须在发布服务前调用。
```

<a id="fn-6c6726c5c280a782"></a>

## `xv::net::Firewall::execute`

```doxygen
@fn int xv::net::Firewall::execute(unsigned operation, firewall_request &request)
@brief 验证版本和结构大小，执行有界链管理或单项规则查询修改。
@details 在服务锁内完成单次原子管理；本层不访问用户地址或凭据，权限由系统调用边界检查。
```

<a id="fn-b1ddf21079255883"></a>

## `xv::net::Firewall::accept`

```doxygen
@fn bool xv::net::Firewall::accept(FilterHook hook, const unsigned char *data, unsigned size, unsigned interface)
@brief 显式映射过滤 hook 与内建入口链，锁外解析报文，锁内完成判决及计数。
@details 无效 hook 或报文解析失败直接拒绝。仅借用报文字节；返回前释放服务锁，由 IPv4 调用方继续处理页面。 实现通过 AcceptFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
```
