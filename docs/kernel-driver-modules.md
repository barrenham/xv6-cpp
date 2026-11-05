# 内核态设备 ELF

用户态负责管理加载和绑定，驱动在内核态直接执行。驱动独立编译成 RV64
ELF，通过版本化 C 函数表调用内核服务，不导入内核私有符号，也不把每次 I/O
转成用户进程消息。模块加载后常驻，目前不支持卸载或替换已绑定实例。

## 默认设备与启动顺序

| 模块 | 源码 | 能力 |
| --- | --- | --- |
| `vblk.elf` | `examples/kernel-driver/virtio_block.cpp` | VirtIO modern 块设备，vda/vdb |
| `serial.elf` | `drivers/backends/serial/` | UART16550 中断输入、排队输出 |
| `rng.elf` | `drivers/backends/entropy/` | random、urandom |
| `mem.elf` | `drivers/backends/memory/` | null、zero、full |
| `gpu.elf` | `drivers/backends/gpu/` | fb0、GPU 队列与完成中断 |
| `input.elf` | `drivers/backends/input/` | kbd0、mouse0、鼠标滚轮 |
| `e1000.elf` | `drivers/backends/e1000/` | PCI e1000 以太网，可多实例 |

这些硬件算法只编译进驱动 ELF，原内核实现已删除或迁出。TTY、PTY、console、
行规程、VFS、网络协议栈、PLIC、中断路由和资源枚举保留在内核。启动日志和
panic 仍需一个最小轮询 UART 入口；正常串口中断和输出队列由 serial ELF 管理。
RTC 和系统定时器也保留为基础设施。

构建主系统会自动构建上述模块。`build/dev/user/image/_fsboot` 是带资源归档的
引导 ELF，携带七个设备 ELF、文件系统后端和 `drivers/boot.list`。QEMU 仍以
`-kernel kernel.elf` 引导内核，`-initrd` 只把引导数据预装入内存。
首个进程首次返回路径先从保留内存加载设备模块，随后初始化网络，再启动用户态
文件系统服务。根盘驱动不依赖根盘文件，因此不会出现先挂载才能加载驱动的循环。

清单每行是归档路径，可附加设备编号；支持空白、空行和 `#` 注释。
默认清单按序加载全部七个模块，并绑定所有匹配且空闲的设备。例如：

```text
drivers/vblk.elf
drivers/serial.elf
# 其余条目见 drivers/boot.list
```

`drivers/vblk.elf 0` 可将启动绑定限制为根盘，给运行中安装第二块盘驱动留出设备。
缺失模块、错误 ELF 或匹配设备初始化失败会终止启动；未提供对应硬件时模块
保持未绑定，不发布该设备节点，也不伪造熵源或 GPU。
软件伪设备没有对应硬件，使用软件资源记录绑定。

## ABI 与调用路径

公共头文件是 `kernel/io/kmodule.h`（ABI 1）和 `kernel/io/kmodule_v2.h`（ABI 2）。
块设备示例保持 ABI 1；其他默认模块采用 ABI 2。入口
`xv_driver_entry(host_abi)` 返回版本化描述，加载器优先请求 ABI 2，再尝试 ABI 1。
ABI 2 增加字符设备会话、串口、以太网、IRQ、poll、用户复制、共享页映射和等待唤醒。
C++ 对象和虚函数表各自留在 ELF 或内核内部，不跨 ABI 传递。

- 内核字符、块、串口和网络代理直接调用模块回调，没有驱动用户进程或每次 I/O 的 IPC。
- IRQ 经 PLIC 和通用路由进入绑定的模块；GPU 初始化期间也能接收完成中断。
- host 提供 DMA 页与资源追踪。模块映像是另行映射的内核虚拟地址，不能直接用作
  DMA 地址；物理页通过 host 分配。网络收发在跨边界交接时明确转移页所有权。
- 等待遵循登记等待通道、释放锁、睡眠和唤醒协议；驱动保留实际设备队列和锁。
- 加载代数记录各 hart 的地址和指令同步状态；回调入口和调度恢复时检查，
  驱动睡眠后迁移 CPU 也能执行新代码，普通 I/O 无需反复刷新全部指令缓存。

ELF 必须是 RV64、LP64 整数 ABI、位置无关的 `ET_DYN`，只允许无外部符号的
相对重定位。不支持 TLS、全局构造/析构或运行时符号解析。加载器检查段边界、
入口、权限、重定位和 ABI，代码 RX、数据 RW、RELRO 只读，不映射 RWX。
模块是受信任的内核代码，错误可以损坏内核，不具备用户进程的故障隔离。
初始化失败须停止 DMA、撤销 IRQ 并释放已申请资源。

当前上限为 32 个常驻 ELF、64 个资源槽，每个文件 256 KiB、映像 1 MiB。
ABI 1 每次绑定最多 32 页；ABI 2 每个 ELF 追踪最多 8192 个数据页，按需分配元数据。
设备树提供 VirtIO MMIO、UART、PLIC、RTC 与 PCI ECAM/内存窗口、中断映射。
内核按这些资源建立映射与中断路由，PCI 通用枚举分配 BAR；驱动按 bus/device_id
匹配。VirtIO 槽位按 MMIO 地址排序，空槽保留资源编号，块设备名称按块设备次序分配。
默认 QEMU 的八个 VirtIO 槽位编号 0..7，UART 为 8、软件资源为 9，PCI 从 10 开始；
应以 `drvctl devices` 的实际结果为准。普通平台设备使用首个 compatible 字符串的
`xv_kmod_compatible_id()` 值作为 device_id。ABI 结构布局保持不变。
不支持运行中硬件热插拔、卸载或重新绑定已占用设备。

## 构建和运行中安装

主构建的块设备目标名为 `driver`，其他目标为 `driver_serial`、`driver_rng`、
`driver_mem`、`driver_gpu`、`driver_input`、`driver_e1000`。
块设备产物为 `build/dev/modules/virtio_block.elf`，其他位于 `build/dev/drivers/`；
除发布 ELF 外还保留 `.debug.elf`。`drivers/sdk/` 提供常用 C++ 接口和 host 桥接，
编译自己的 PIC 对象，不链接内核二进制对象。

运行中安装新块驱动仍可独立于主构建进行：

```sh
cmake -S examples/kernel-driver -B build/my-driver -G Ninja -DXVK_NAME=mydisk
cmake --build build/my-driver -j 1
python3 -m http.server 8000 --directory build/my-driver --bind 0.0.0.0
```

需要支持 RISC-V 的 clang++ 和 ld.lld。示例只依赖公共 ABI 1 头文件，可将
示例与头文件拷出仓库，指定 `-DXVK_ABI_DIR=/path/to/header`；也可用
`-DXVK_SOURCE=/path/to/driver.cpp` 指定自己的源文件。

来宾 root 可执行：

```sh
drvctl devices
drvctl list
curl -fsS -o /tmp/mydisk.elf http://10.0.2.2:8000/mydisk.elf
drvctl load /tmp/mydisk.elf
# 默认自动绑定匹配的空闲设备；显式手动绑定可使用：
drvctl load --no-bind /tmp/other.elf
drvctl bind MODULE DEVICE
# 对已加载的模块再次扫描空闲设备：
drvctl bind --auto MODULE
```

命令中的 MODULE/DEVICE 编号须按查询结果替换；`10.0.2.2` 是 QEMU user 网络的宿主地址。
默认启动已绑定所有匹配设备，新模块需要空闲且匹配的资源。
可用前述仅根盘绑定清单在测试中保留设备 1，再在线绑定新 ELF。
第二块磁盘成功绑定后发布 vdb，已有用户态 FS 可用 `mount -t ext3 -s 1 PATH`
挂载有效文件系统。无需重启或重新编译内核、fsboot。
管理入口 syscall `kmod` 和 `drvctl load/bind` 均检查有效 UID 0。

## 验证和维护

`device_elf_boundary` 检查实际内核编译输入、内核符号及七个 ELF，确认具体硬件
实现未链接进内核、ELF 无未解析导入、引导归档与当前构建产物逐字节一致。
`kernel_driver_boot_{1,5}cpu` 检查七模块的启动加载顺序、ABI、多设备绑定和存储读写。
`kernel_driver_hotplug_{1,5}cpu` 在登录后才独立编译新名称 ELF，再经真实网络下载、
在线加载和绑定；检查权限、畸形 ELF、64 次失败绑定回收、四进程并发读及 supervisor
指令执行，并离线检查 ext3 落盘与一致性，确认内核和 fsboot 没有被改写或重启。

设备专项覆盖 devfs、缺失硬件、GPU 共享帧缓冲、VNC、键鼠滚轮、串口/PTY、
单核/五核网络、多网卡和文件系统块能力。测试使用私有磁盘副本，不更新持久根盘。
格式由 `.clang-format` 固定：属性、返回值、函数定义分行，函数左花括号在签名行，
每行只放一条语句。

2026-11-04 验收：34 项设备、网络、图形、启动与 FS 集成回归的最终结果全部通过。
日志为 `build/device-elf-final-tests.log` 和 `build/device-elf-final-recheck.log`；
后者包含截断 ELF 测试输入修正后的四项复验，以及同时提供全部设备的单核/五核启动验收。
最终结果与内核、引导归档、七个模块的 SHA-256 汇总在
`build/device-elf-acceptance.json`。正常镜像构建、C++ 格式检查和 clangd 的
诊断、跳转、补全检查通过，编辑器日志见 `build/device-elf-clangd-tests.log`。

`drvctl list/devices` 默认显示带表头的对齐表格和字段说明。脚本可使用 `drvctl --raw list`、`drvctl --raw devices` 保留原键值输出。MODULE 与 DEVICE 是绑定命令的两个编号，DEVICE_ID 是总线类型标识，详见 [命令输出说明](command-output.md)。

设备树与自动绑定的扩展回归：`device_tree_contract` 使用真实 QEMU DTB，修改
MMIO/IRQ 验证解析随资源变化并拒绝畸形资源；`device_discovery_{1,5}cpu` 将
根盘/熵源放在第 7、8 个 VirtIO 槽位，并验证在线加载后自动绑定第二盘、挂载读写、
重复自动扫描不抢占已有设备。测试均使用临时磁盘。

早期发现代码在 4 KiB 固定启动栈上执行。资源描述必须原地初始化，避免生成超过
6 KiB 的聚合临时对象；该源文件启用 `-Wframe-larger-than=2048` 并由 Werror 拒绝
过大的单函数栈帧。完整启动测试补充检查跨函数调用和实际驱动路径。
