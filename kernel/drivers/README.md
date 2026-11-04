# 内核设备框架

`import xv.drivers;` 提供设备公共接口。常用硬件实现已迁入独立内核态 ELF，
启动时从内存归档加载，详细契约见 [内核态设备 ELF](../../docs/kernel-driver-modules.md)。

| 位置 | 职责 |
| --- | --- |
| `modules/runtime.cpp` | ELF 校验、映射、相对重定位、启动清单和运行中绑定 |
| `modules/services.cpp` | 字符、串口、网络代理和版本化 C ABI host 服务 |
| `modules/resources.cpp` | 常驻 ELF 的物理页所有权与回收 |
| `board/qemu_virt.cpp` | 资源枚举、早期控制台、通用实例装配和 IRQ 路由 |
| `interrupt/` | PLIC 和通用中断路由 |
| `tty/` | TTY、PTY、行规程、回显、termios 和等待协议 |
| `console/`、`block/`、`serial/`、`net/` | 公共设备能力接口 |
| `rtc/`、`platform/` | RTC、内核等待/复制/DMA 等基础设施 |
| `../../drivers/backends/` | UART、熵源、GPU、键鼠、内存设备和 e1000 的实际实现 |
| `../../examples/kernel-driver/` | 实际启动使用的独立 VirtIO modern 块设备驱动 |
| `../../drivers/sdk/` | 驱动侧 host 桥接与 VirtIO 协议常量 |

具体算法不再编译进内核。内核保留最小 UART 轮询入口，供模块装载前的日志和
panic 使用；正常收发由 serial ELF 实现。Goldfish RTC 与定时器仍为内核基础设施。

首个进程在用户态文件系统服务启动前加载七个默认模块并绑定设备，随后启动网络。
模块可以同时绑定多个硬件实例；无对应硬件时不发布节点。根磁盘驱动来自引导
归档而非根盘，内核不需要内置块驱动来读取模块。

文件系统通过 CharacterProxy/ModuleBlock 等通用代理调用 ELF 中的 C 回调。
TTY 只依赖 SerialPort 和 InputReceiver，网络协议栈只依赖 EthernetDevice；
中断从 PLIC 经板级路由进入绑定的模块。不在每次 I/O 创建用户态消息。
模块可阻塞，host 提供锁所需的 CPU、中断、等待和唤醒操作；跨 CPU 调度恢复
执行按加载代数的代码同步。DMA 页的分配与交接由 host 追踪，模块映像本身不能
当作物理 DMA 内存。

TTY、PTY 的寿命、控制终端和行规程见 [TTY 说明](tty/README.md) 与
[devfs](../../docs/devfs.md)。显示/键鼠接口见 [图形](../../docs/graphics.md) 和
[键盘](../../docs/keyboard.md)，这些接口保持在设备 ELF 之后供 fbterm 使用。

`driver_contract` 使用迁移后的同一份源代码验证串口、PLIC、熵源、GPU、键鼠、
TTY、PCI 配置等协议。块设备现代队列由真实 QEMU 启动和在线加载测试验证，
删除的旧块驱动不再保留另一份宿主机实现。`device_elf_boundary` 检查最终二进制
与引导资源，QEMU 测试覆盖单核/五核、设备缺失、网络、显示和文件系统访问。
