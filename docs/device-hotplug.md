# 设备热插拔

系统在运行中可以发现预留 PCIe 插槽中的设备，匹配已加载的驱动 ELF，并在有序拔除时撤销网络接口、停止回调和 DMA、回收实例资源，再关闭插槽电源。首个完整适配的设备是 e1000 网卡。

## 直接使用

开发用 `qemu`、`qemu-vnc`、`qemu-graphics` 目标默认预留 `hotplug0`、`hotplug1` 两个 PCIe Root Port，提供本地 QMP socket `build/dev/qemu.qmp`。更新根盘后重新启动 QEMU，在宿主机的另一个终端运行：

```sh
python3 tools/device_hotplug.py add-e1000 extra0 --slot hotplug0
python3 tools/device_hotplug.py remove extra0
```

系统内以 root 查看：

```sh
drvctl hotplug              # 当前状态，包括空插槽和拒绝原因
drvctl events               # 最近的事件
drvctl watch                # 持续查看；Ctrl-C 退出
netctl link
netctl addr
```

添加设备无需手工执行 bind/rescan。默认启用 DHCP 的构建会为新网卡启动 DHCP；关闭该构建选项时，可手工配置地址或运行 `netctl dhcp start ethX`。主机工具为热插网卡选择与启动网卡不同的子网，默认 hotplug0 为 10.0.3.0/24、hotplug1 为 10.0.4.0/24。

```sh
# 主机：指定地址和子网
python3 tools/device_hotplug.py add-e1000 extra1 --slot hotplug1 --net 10.0.8.0/24 --mac 52:54:00:12:34:99
python3 tools/device_hotplug.py list

# 系统内：立即扫描，或主动安全拔除指定 DEVICE
# DEVICE、GEN 从 drvctl hotplug 的对应列读取
drvctl rescan
drvctl remove DEVICE
drvctl remove DEVICE GEN
```

`remove` 等待客户机实际完成拔除，而非仅接受 QMP 请求。超过等待时间会提示检查系统内的状态和事件。`drvctl remove` 默认先读取并携带当前 GEN，避免查询和操作之间插入了另一代设备；显式提供旧 GEN 会被拒绝。

其他构建目录可指定 `--socket build/其他预设/qemu.qmp`。CMake 的 `XV_HOTPLUG_SLOTS` 可设为 0–3，`XV_QMP_SOCKET` 可指定 socket 路径。0 个插槽时不创建热插拔轮询任务。

## 状态与事件

| 列 | 含义 |
| --- | --- |
| DEVICE | 内核资源编号；预留插槽反复插拔时编号不增长 |
| GEN | 设备代次；新设备插入同一槽位时递增 |
| PRESENT | 当前槽位中是否有已发现的设备 |
| STATE | 绑定、未绑定、操作被拒绝或移除失败隔离状态 |
| IFACE | 当前分配的网络接口；拔除后该接口槽位可复用 |
| REMOVAL | 支持拔除、受保护或驱动缺少移除回调 |

事件包括 ADDED、BOUND、REMOVED、REJECTED、FAILED。队列固定容纳 64 条，使用单调递增序号；`drvctl events AFTER` 从给定序号后继续读取。读取落后超过容量时，首条结果的 LOST 显式报告漏掉的条数；客户端应重新查询 hotplug 状态后继续。

根盘、控制台、其他静态平台设备以及未适配移除的驱动受保护。移除回调无法停止设备时，内核保留资源并隔离后续调用，显示 FAILED，等待 `rescan` 或 `remove` 显式重试，不持续重复释放资源。

## 内核与驱动约定

- 总线层为单级 PCIe Root Port 分配次级总线和每槽 2 MiB 的 32 位 MMIO 窗口，按 DT 的 PCI INTx 路由预先配置各 hart 的 PLIC。窗口按槽位复用，避免每次插入重新消耗地址空间。
- `device-hotplug` 内核任务按 250 ms 的定时等待检查插槽，不在等待期间空转。空插槽的注意事件和已接入设备的拔除请求按当前状态区分。W1C 状态只确认实际读到的事件位；实现对照 [QEMU PCIe 插槽状态机](https://github.com/qemu/qemu/blob/v8.2.0/hw/pci/pcie.c)，实际验证使用本机 QEMU。
- `kernel/io/kmodule_v4.h` 扩展驱动描述符，加入 remove(host, identity, reason) 回调。ABI 1/2/3 的 ELF 继续可以加载；它们没有此回调，不能进行有序解绑。
- 内核先阻止新的网络回调，排空已有调用与中断处理；随后撤销接口，清理 DHCP、地址、路由、ARP 待发页和网桥成员/FDB，再调用驱动停止 DMA 和释放本实例资源，最后关闭插槽并确认事件。
- e1000 驱动关闭中断、收发及设备运行状态，复位并确认停止后，释放 RX、在途 TX 和实例页。每个绑定独立管理实例，移除一个网卡不会释放同一 ELF 所管理的其他网卡。
- ELF 代码保持驻留；本次回收的是绑定的实例和 DMA 资源。网络代理与接口骨架保持稳定，重接只刷新 MAC、ARP 和配置，避免陈旧指针以及重复初始化正在使用的锁。
- 移除回调失败必须保留资源；成功必须保证设备不再 DMA，也不会再引用实例。回调接收的 identity 仅在调用期间有效，binding token 标识本次实例。`XV_KMOD_REMOVE` 的 value 为预期代次，0 保留为不指定代次的兼容调用。

本次适配范围为 QEMU RISC-V virt 的 DT/ECAM、单级 Root Port 下的 function 0 e1000 端点。USB、SATA、多级 PCIe 交换机和字符/块设备端点的可撤销发布不在本次支持范围内。验收覆盖由 QMP 或客户机主动请求并完成的有序拔除；真实硬件突然断电、DMA 故障等路径仍需硬件专项验证。

## 验证

`device_hotplug_1cpu/5cpu` 使用独立磁盘、实际 QMP device_add/device_del：反复插入、自动匹配、DHCP/ping、IRQ 计数、稳定编号与递增代次、旧代次拒绝、接口和路由清理、原根盘继续工作、空闲内存与事件队列上限。五核用例还保留启动网卡，检查同一驱动的多实例，并通过第二个插槽测试共享 INTx 和客户机主动安全拔除。

原生 network_contract 验证网络接口撤销、地址关联路由清理和 32 次接口重接后的数据页回收。已有驱动引导、依赖、设备枚举、网络和网桥回归用于检查兼容性。测试盘正常 sync 后运行只读 e2fsck。日志和测量保存在构建目录的 `hotplug-*.log`、`device-hotplug-{1,5}cpu.log/json`。

### 本次结果（2026-11-08）

QEMU 9.2.1，128 MiB 内存；两组热插拔和九项原有驱动、设备、网络相关检查全部通过。每组另完成一次第二插槽的客户机主动拔除。

| CPU 数 | 主插槽插拔轮数 | 拔除后最低可用 KiB | 最高可用 KiB | 主插槽代次 |
| --- | --- | --- | --- | --- |
| 1 | 24 | 124948 | 124948 | 2 → 25 |
| 5 | 24 | 123692 | 123692 | 2 → 25 |

逐轮采样未出现可用内存下降。多核组保留启动网卡，验证移除其他实例后仍可通信。原生测试另外覆盖 32 次接口重接。完整结果分别见构建目录 `hotplug-final-validation.log`、`hotplug-regression-validation.log`；前一日志包含修正测试夹具后的最终四项结果。
