# E1000 与 UDP 网络

网络部分参考相邻 `xv6-labs-2025` 的 `origin/net` 实验布局（`982b43b`）。该仓库已切换到本地跟踪分支 `net`，原有未跟踪文件保留。原分支的 E1000 收发、UDP 绑定和接收存在待实现部分，本项目补齐并改为 C++20 模块及当前驱动/进程架构。

## 运行与测试

```bash
conda activate OS-LAB
cmake --preset dev
cmake --build --preset dev
ctest --test-dir build/dev --output-on-failure -R '^network_'
cmake --build build/dev --target qemu
```

启动会显示 `e1000: ready (10.0.2.15)`。手动启动 QEMU 时需提供：

```text
-netdev user,id=net0
-device e1000,netdev=net0,bus=pcie.0,addr=01.0,mac=52:54:00:12:34:56
```

`qemu` / `qemu-gdb` 目标默认带上述参数。`nettest` 默认访问宿主机 UDP 26099，也可运行 `nettest <宿主机端口>`；它需要一个回显服务。宿主机另一个终端运行：

```python
import socket
with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as peer:
    peer.bind(('127.0.0.1', 26099))
    while True:
        data, address = peer.recvfrom(4096)
        peer.sendto(data, address)
```

自动测试会启动独立回显服务，选择临时端口，使用临时磁盘并结束 QEMU。单核和三核各在同一次启动中运行两轮，验证环回绕、1472 字节负载、空报文、截断接收、非法地址、fork 共享、退出清理、EINTR 与 SA_RESTART。

入站连接需要显式配置 slirp 端口转发，例如把宿主机 26000 转发到客体 UDP 2000：

```bash
cmake --preset dev '-DXV_NET_BACKEND=user,id=net0,hostfwd=udp:127.0.0.1:26000-:2000'
cmake --build build/dev --target qemu
```

恢复默认：`cmake --preset dev -DXV_NET_BACKEND=user,id=net0`。修改客体 IP/MAC 时须同步协议栈、驱动接收地址和 QEMU 参数。

## 模块与数据流

| 路径 | 职责 |
| --- | --- |
| `kernel/drivers/net/ethernet.cppm` | EthernetDevice / EthernetReceiver 的提交与页面所有权契约 |
| `kernel/drivers/net/e1000/` | Intel 82540EM 寄存器、DMA 收发环、完成回收与中断处理 |
| `kernel/drivers/pci/` | QEMU virt 固定 PCI 插槽的标识检查、BAR0 与总线主控配置 |
| `kernel/drivers/board/qemu_virt.cpp` | 网卡静态实例及 IRQ 33 的 PLIC 路由装配 |
| `kernel/net/` | `xv.net`：ARP 应答、IPv4/UDP 校验、组包、端口与接收队列 |
| `kernel/net/platform/` | 物理页、当前 PID、用户复制及可中断等待的内核适配 |
| `kernel/syscall/sysnet.cpp` | 预留网络系统调用的用户参数检查 |
| `tests/network_contract.cpp` | 原生协议/资源边界测试，注入短包、错误校验和及队列溢出 |
| `tests/network.py` | 真实 QEMU E1000 与本机 UDP 往返测试 |
| `user/tests/nettest.cpp` | 客体内回显与进程/信号生命周期测试 |

TX：用户复制 → 协议组包 → transmit → DMA → DD → 释放。提交失败时页面仍属调用者；成功后驱动持有页面，只有硬件确认完成后才能归还。16 项发送环留一个空槽，环满立即返回 `-EAGAIN`。

RX：DMA 写入驱动页 → DD/EOP 检查 → 分配替换页并重设 RDT → 释放设备锁 → 协议检查 → 入队或释放。内存不足时复用原 RX 页并丢包；不让设备继续写已经移交给用户接收队列的页面。回调不持设备锁，ARP 应答可安全再次进入发送路径。

描述符环按 128 字节对齐，使用完整 64 位 DMA 地址；发布字段后执行 I/O 屏障。收包、发包和队列操作通过自旋锁保护。接收睡眠在队列锁内登记等待、解锁后让出 CPU，避免丢失唤醒。

## 用户态接口

这些接口沿用实验的 UDP 端口 API，地址/端口数值使用主机字节序；IPv4 例如 `0x0a000202` 表示宿主机网关 `10.0.2.2`。

```cpp
int bind(uint16 port);
int unbind(uint16 port);
int send(uint16 source_port, uint32 destination_ip, uint16 destination_port,
         const char *data, uint32 length);
int recv(uint16 port, uint32 *source_ip, uint16 *source_port,
         char *data, uint32 capacity);
```

- `bind` 独占创建全局端口，成功返回 0；端口 0、重复绑定、资源不足和无设备返回 -1。
- `fork` 共享父进程持有的绑定及队列；不同持有者竞争消费。`unbind` 只释放当前进程的引用，最后一个引用消失才释放端口和排队页。退出和信号终止自动清理；绑定在同一 PID 的 exec 后仍保留。
- 全局最多 32 个端口、256 个进程持有引用；每端口最多 16 个数据报。队列满丢弃新到数据报，不影响其他端口的配额。
- `send` 不要求源端口已绑定；成功提交返回 0，负载最多 1472 字节，允许空数据报。环满返回 `-EAGAIN`，其他失败返回 -1。提交成功不等于对端收到，UDP 不提供可靠性。
- `recv` 等待一个完整数据报并返回复制字节数；容量不足时截断并丢弃剩余部分。用户地址失败会消费该报并释放页面。空报返回 0。捕获信号可使等待返回 `-EINTR`；处理动作设置 `SA_RESTART` 时自动恢复等待。

## 当前范围

板级目标为 QEMU virt 的 bus 0、slot 1、function 0，Intel 8086:100e，BAR0 `0x40000000`，IRQ 33。没有网卡时仍可启动，其余系统功能正常，网络操作失败。

协议栈固定客体 `10.0.2.15`、MAC `52:54:00:12:34:56`，出站帧交给 QEMU slirp 网关 MAC `52:55:0a:00:02:02`。每次合法 ARP 请求都会应答。IPv4 检查版本、首部/总长度、TTL、分片标志和校验和；UDP 检查长度和非零校验和，发送时生成 UDP 校验和。按现有实验范围，不实现 DHCP、动态路由/通用 ARP 解析、IP 分片重组、ICMP、TCP、IPv6 或 socket 文件描述符，因此暂不提供网络 `poll`/`fcntl`。

函数级 Doxygen 仍放在 [网络函数导航](functions/kernel/net/stack.cpp.md) 和 [E1000 函数导航](functions/kernel/drivers/net/e1000/e1000.cpp.md)，经现有外部索引生成工具供 clangd 原生 hover 使用。
