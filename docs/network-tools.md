# 网络测量与终端命令

## ping

```sh
ping -c 4 127.0.0.1
ping -c 5 -i .2 -W 1 -s 56 10.0.2.2
ping -q -c 10 example.com
```

`ping` 发送 IPv4 ICMP Echo Request，核对 Echo Reply 的来源、标识符、序号、长度、校验和与负载，报告 RTT、收包率和丢包率。RTT 使用 10 MHz `clockticks()` 计时，按微秒计算并显示毫秒。支持 DNS 名称，采用解析出的第一个 IPv4 地址；公网 Echo 能否到达取决于宿主机网络。

| 参数 | 含义 |
| --- | --- |
| `-c COUNT` | 发送 1..65535 个请求；缺省持续运行，Ctrl-C 后输出统计 |
| `-i SECONDS` | 相邻请求的目标间隔，默认 1 秒 |
| `-W SECONDS` | DNS 解析总时限和每个请求的等待时限，默认 1 秒 |
| `-s BYTES` | Echo 数据长度，0..1400，默认 56；不含八字节 ICMP 头 |
| `-q` | 仅打印启动信息和最终统计 |
| `--` | 后续参数按主机名解释 |

时间参数接受正十进制秒，最多三位小数，范围 .001..3600。每次仅有一个在途探测；应答慢于间隔时，下次发送等待当前探测完成。等待通过 poll 睡眠，调度与定时中断会影响实际唤醒精度，毫秒参数不承诺毫秒级调度。

每个有效请求最多计入一次 received。等待后续请求时，可识别最近一次成功请求的重复应答并单独计数；这不是无限历史去重。超时后到达的旧请求应答不计入 received。丢包率为整数百分比；RTT 统计只包含首次有效应答。退出码：有应答为 0，全部无应答为 1，参数、解析或传输错误为 2。Ctrl-C 也按已有应答决定状态。

目前只支持 IPv4 Echo，不报告 TTL、ICMP 不可达详情或路径 MTU，也不提供 traceroute。收到非 Echo Reply 或校验失败报文时继续等待，最终可能超时。

## 内核接口与资源

用户接口为 `socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP)`，支持原有 bind/connect/sendto/recvfrom、非阻塞、poll 与 getsockname。接收内容从 ICMP 头开始，不包含 IPv4 头。只允许发送 type 8/code 0、长度 8..1472 的报文，内核重写 Echo 标识符和校验和，不开放任意原始 IP 报文。

`DatagramSocket` 复用原 UDP 数据报实现；保留原 `udp_socket` 模块分区和流程文件路径。protocol 17 维持 UDP 语义，protocol 1 使用本地端口号作为 Echo 标识符，不要求对端端口。注册表按协议隔离绑定，协议内标识符唯一；Echo Reply 在注册表锁内按标识符、本地地址和已连接对端匹配。原 64 个 socket 上限与每 socket 16 条接收队列仍适用。未接收、校验失败或满队列的报文由原 Packet 所有权机制回收。

ICMP 复用 IPv4 路由、ARP、防火墙、回环与网卡发送路径；原 Echo 应答功能继续存在。此项是对之前网络流程保留基线的明确扩展，源清单重新登记当前哈希，原图仍保留。新增接口契约与有界报文编码作为原语记录在 network-flow-coverage.json。

## clear

```sh
clear
clear -x
```

默认向标准输出写入 `ESC[H ESC[2J ESC[3J`：光标归位、清屏并请求清除终端滚动历史。`-x` 省略最后一项，保留历史。实际效果由连接的终端是否支持这些控制序列决定；重定向时写出同样的原始字节。成功返回 0、输出失败返回 1、参数错误返回 2。

## 验证

`network_contract` 验证受限 Echo socket、标识符隔离、奇数字节校验、重复收发与页面回收。`http_contract` 另外覆盖共享时间参数边界与 Echo 报文核对。

`http_1cpu` 和 `http_5cpu` 在实际客体中覆盖 curl、ping 与 clear：回环零字节/默认/1400 字节负载、quiet、Ctrl-C、防火墙确定性丢包、ANSI 字节。第二块 E1000 连接宿主可控以太网对端，注入重复应答、丢包及校验和有效但标识符或负载错误的应答，验证匹配与统计，不依赖公网 ICMP 权限。

curl 的优化和测量字段见 [HTTP 说明](network-http.md)。联合回归仍保留既有 fork 五秒性能门禁，未调整阈值。

本轮结果与测量边界记录在 [验收记录](network-tools-acceptance.json)，本机详细日志保存在 `build/docs/network-tools-final/`。
