# 用户态网络管理：netctl

`netctl` 使用版本化的系统调用管理当前运行中的网络配置。物理与隧道接口对象仍在启动时创建；网桥支持运行期发布与注销，见 [软件网桥](network-bridge.md)；接口地址、管理状态和路由可以在运行中修改。设置只保存在内存中，重启后恢复启动策略。

## 常用命令

```sh
netctl link
netctl addr
netctl route
netctl neigh
netctl stats

netctl link set eth0 down
netctl link set eth0 up
netctl addr add 10.0.2.16/24 dev eth0
netctl addr show eth0
netctl addr del 10.0.2.16/24 dev eth0

netctl route add 192.0.2.0/24 dev eth0 via 10.0.2.2 metric 7
netctl route del 192.0.2.0/24 dev eth0 via 10.0.2.2 metric 7
netctl route add default dev eth0 via 10.0.2.2
netctl neigh flush eth0
```

查询支持 `show` 和可选的接口筛选，例如 `netctl stats show eth0`。路由命令另支持 `src IP`，供指定源地址和无编号三层接口使用。所有 IPv4 输入严格检查四个十进制字节和 0..32 的前缀，数字溢出、未知选项和重复选项会报错。删除静态路由时，目标前缀、出口、网关、metric 和源地址必须与添加时一致；省略项分别为零。

接口列表区分管理状态与物理载波：`DOWN carrier up` 表示设备链路仍连通，但管理上禁止使用。无 MAC 的回环和隧道不会显示虚构 MAC。以太接口可以删除最后一个 IPv4 地址，设备、MAC 和接口身份仍保留。当前保留启动回环的地址和启用状态，修改 `lo` 返回不支持。

DHCP 地址和路由带独立归属，不能通过普通删除命令移除；应使用 `netctl dhcp stop DEV`。停止或过期保留手工配置。

添加手工地址会同时生成到本机地址的 /32 回环路由及接口直连前缀路由。容量检查与这三项修改在同一配置事务内完成，失败不留下部分配置。标有 `automatic` 的路由归地址所有，不能单独删除。删除地址同时清除其自动路由，以及显式以该地址作为首选源的路由。多个地址可以拥有相同的直连前缀，各自的自动路由独立归属。

路由记录的存储、压缩、归属检查和静态路由校验统一由 `RoutingTable` 负责。管理层通过 `at()` 取得单条值快照，通过 `add_static()` / `remove_static()` 修改手工路由，通过地址路由操作维护自动路由；不访问路由数组，也不临时解除启动封存。地址与路由联合更新时，外层 `ConfigurationGuard` 仍覆盖完整事务，路由方法内部的 Guard 借用已有锁。

手工地址的字段校验、全局地址冲突、容量预检和原子提交由 `AddressConfiguration::add_static()` / `remove_static()` 负责。这个服务也供 DHCP 地址安装复用，只有它与接口注册表具备接口私有配置状态的访问权限。直接使用服务时会核对接口的注册对象身份；与注册对象 ID 相同的其他对象也不能修改配置。

`NetworkInterface::set_up()` 与 `add_address()` 保留启动封存语义；运行期启停使用 `configure_link()`，每次成功管理请求推进接口代次。DHCP 活动标记使用 `set_dhcp_active()`，保持原有的不改变代次行为；网桥成员关系变化使用 `topology_changed()` 推进代次。需要的邻居和待发队列清理仍在配置锁外进行。

网关必须位于出口的某个本地前缀内，且不能是自身或接口广播地址。以太出口的首选源必须属于该出口；三层出口可显式借用其他本机地址。最长前缀与较小 metric 规则不变。删去某地址后，剩余网关路由若已无可用本地前缀，会暂时失效；恢复合适地址后可重新生效。

## 已有 socket 和在途报文

配置操作不会隐式关闭已有 socket。已绑定或已连接 socket 保留原本的本地地址与端口；关闭接口、删除源地址或删除所需路由后，新发送会重新选路并返回 `-ENETUNREACH`。恢复原配置后，同一 UDP socket 可以继续通信。TCP 保留重传状态，后续重传重新选路；不可达持续过久仍会按原有重传上限超时。

已经交给 DMA 或已经进入 socket 接收队列的数据可以继续完成或被读取。配置改变不撤回硬件已经接受的报文，也不清空应用已接收的数据。接口停用或改地址会清空该接口的邻居及 ARP 待解析队列；待解析报文还携带配置代次，避免与清空操作并发入队的旧报文稍后按新配置发送。回环排队的报文交给 IP 时仍检查当前本地地址。

未连接、绑定 ANY 的 UDP 继续逐包选源；已连接 UDP 和 TCP 不会因为增删地址而自动迁移本地端点。运行期创建/删除目前只支持两个网桥槽；DHCP 已支持自动取址和续租，见 [DHCP 说明](network-dhcp.md)；尚未实现通用接口创建、改变 MTU、配置隧道端点或磁盘配置持久化。

## 管理 ABI

用户包含 `user/runtime/user.h`，调用：

```cpp
netctl_request r{};
r.version = NETCTL_VERSION;
r.size = sizeof(r);
r.index = 0;
int result = netctl(NETCTL_LINK, &r);
```

ABI 定义在 `kernel/net/control.h`，版本一结构固定为 128 字节。查询成功返回 1，枚举结束返回 0，修改成功返回 0，失败直接返回负 errno。没有 errno TLS。

| 操作 | 输入 | 输出 |
| --- | --- | --- |
| `NETCTL_LINK` | ifindex 非零按 ID 查询；否则按 index 枚举 | 名称、MTU、地址数、管理/载波/MAC/隧道标记及计数 |
| `NETCTL_ADDRESS` | ifindex、index | address、prefix |
| `NETCTL_ROUTE` | index | 出口 ifindex、address/prefix、gateway、metric、source、自动标记 |
| `NETCTL_NEIGHBOR` | ifindex、index | IPv4、MAC、剩余有效毫秒 expires_ms |
| `NETCTL_SET_LINK` | ifindex、flags 为零或 `NET_LINK_UP` | 无 |
| `NETCTL_ADD_ADDRESS` / `NETCTL_DEL_ADDRESS` | ifindex、address、prefix | 无 |
| `NETCTL_ADD_ROUTE` / `NETCTL_DEL_ROUTE` | ifindex、address/prefix、gateway、metric、source；flags 为零 | 无 |
| `NETCTL_FLUSH_NEIGHBORS` | ifindex | 无 |

地址字段使用主机字节序。调用者必须提供正确 version/size；未知版本或大小返回 `-EINVAL`，非法用户指针返回 `-EFAULT`。其他常见错误为不存在 `-ENOENT`、未知接口 `-ENODEV`、重复 `-EEXIST`、容量不足 `-ENOBUFS`、自动路由忙 `-EBUSY` 和不支持 `-EOPNOTSUPP`。本教学系统没有用户权限分级，管理入口对所有进程开放。

接口和路由查询在配置锁下生成单条记录，再在锁外复制到用户地址；没有内核指针或未初始化填充暴露给用户。整轮枚举不冻结其他进程的修改，并发增删时可能需要重新查询。邻居记录在本接口 ARP 锁下单独取得。

计数按纯 IP 接口边界统计。RX 计数是接口向 IP 层投递的次数及字节，RX dropped 包含接口禁止投递及 IP 基础验证拒绝；不覆盖所有驱动、ARP 或传输层丢弃。TX 计数是出口已接受的包和字节，包含等待 ARP 的包；TX errors 是出口 transmit 返回负值的次数。这些数值不代表硬件实际发出的以太帧数量。每个计数原子读取，六个字段并非一个时刻的联合快照。

## 内核结构与锁

`NetworkManager` 分派管理请求，`AddressConfiguration` 协调地址与路由事务，`ConfigurationGuard` 为配置读写提供中断安全的短临界区。同一调用链的嵌套配置查询复用已持有的锁；路由查询可据此读取一致的地址、状态和源约束。接口地址查询返回值副本，避免并发删除后悬空引用。

配置锁内不进行用户复制、睡眠、协议回调、socket 等待或获取 ARP/驱动锁。管理操作先发布新的配置及代次，解锁后清空 ARP；ARP 路径可以持邻居锁短暂读取配置，形成单向锁关系。收发回调在配置锁外运行，避免与 socket 注册锁产生反向依赖。物理接口注册、接收器绑定和对象存储仍在启动期确定；网桥使用长期存储的固定槽，在运行期发布或注销，时钟回调使用受锁保护的指针快照。

## 验证

```sh
ctest --test-dir build/dev --output-on-failure -R '^(network_contract|netctl_[15]cpu)$'
```

原生契约检查 ABI、自动路由归属、容量不足时的事务回滚、并发查询与地址变更、ARP 待发页释放，以及删除最后一个 IPv4 后的设备保留。单核和三核 QEMU 测试从真实 shell 执行 CLI，并在同次启动重复运行系统调用测试：真实 UDP 往返、邻居查询/清空、停用/恢复、删地址/恢复后复用原 socket，以及一个进程修改物理配置时另一个进程保持回环通信。

接口基础设计见 [网络接口与隧道](network-interfaces.md)，socket 行为见 [网络开发说明](network.md)。
