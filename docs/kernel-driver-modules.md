# 运行时加载内核驱动

用户态管理加载和绑定，模块在内核态执行。新驱动独立编译为 RV64 ELF，
通过版本化 C 函数表接入设备层，不链接内核私有 C++ 类或符号。
加载后常驻，本版本不提供卸载。

## 运行中安装

构建主系统并启动一次后，在另一终端编译驱动：

```sh
cmake -S examples/kernel-driver -B build/my-driver -G Ninja \
  -DXVK_NAME=mydisk
cmake --build build/my-driver -j 1
python3 -m http.server 8000 --directory build/my-driver --bind 0.0.0.0
```

编译器需要支持 RISC-V 的 clang++ 和 ld.lld；项目中的 OS-LAB 环境可用。
独立项目只编译模块源文件，依赖的唯一内核头文件是 `kernel/io/kmodule.h`。
也可将示例目录与该头文件拷出仓库，配置 `-DXVK_ABI_DIR=/path/to/header`。
编写其他驱动时通过 `-DXVK_SOURCE=/path/to/driver.cpp` 指定源文件。

QEMU 配置第二块 VirtIO MMIO 磁盘在 `virtio-mmio-bus.1` 后，来宾 root 执行：

```sh
drvctl devices
drvctl list
curl -fsS -o /tmp/mydisk.elf http://10.0.2.2:8000/mydisk.elf
drvctl load /tmp/mydisk.elf
drvctl bind 0 0
drvctl list
ls /dev
mkdir /tmp/volume
mount -t ext3 -s 1 /tmp/volume
```

这里 `0 0` 表示模块编号 0、设备编号 0；以实际查询结果为准。
`10.0.2.2` 适用于 QEMU user 网络；其他网络环境使用可达的主机地址。
第二块磁盘必须已经具有对应文件系统。上例不会格式化磁盘。

内核在绑定之前不初始化第二块磁盘，也不发布 `/dev/vdb`。绑定成功后，
原有用户态文件系统 ELF 通过块设备能力访问它。无需重启，也无需重编译
内核或 fsboot。启动磁盘、控制台等原有关键驱动仍内置。

## 模块契约

- ABI 1 定义在 `kernel/io/kmodule.h`，管理入口为 syscall `kmod`。
  `drvctl` 提供 `load`、`list`、`devices`、`bind`；管理操作要求有效 UID 0。
- 模块为 RV64、LP64 整数 ABI、位置无关的 `ET_DYN` ELF，入口
  `xv_driver_entry(host_abi)` 返回驱动描述。模块可用 C++ 编写，但没有用户态
  运行库、系统调用、TLS、全局构造/析构和隐式符号解析。
- 加载器验证段布局、大小、权限和 ABI，只接受无外部符号的
  `R_RISCV_RELATIVE` 重定位。完成重定位后代码页 RX，数据页 RW，RELRO 页 R；
  不创建 RWX 映射。各调用 CPU 在进入模块前执行地址翻译与指令同步。
  使用加载代数记录每颗 CPU 的同步状态；调度恢复路径也检查代数，因此驱动
  睡眠、抢占后迁移到其他 CPU 时仍能正确执行，普通 I/O 不重复刷新指令缓存。
- `bind` 获得明确的 MMIO/IRQ 资源描述和 host 函数表；可分配零填充的 DMA 页、
  创建锁、原子等待和唤醒。失败必须停止设备 DMA，主机回收本次绑定的资源。
- 驱动返回块设备读写、flush、中断回调及容量。内核直接调用这些函数，
  没有用户态驱动进程，也不对每个 I/O 发送跨进程消息。
- 块设备缓冲区与 host DMA 页是物理地址恒等映射的 RAM；模块映像本身是
  另外映射的内核虚拟地址，不可把模块虚拟地址直接交给硬件 DMA。

ELF 约定参考 [RISC-V ELF psABI](https://riscv-non-isa.github.io/riscv-elf-psabi-doc/)。

## 当前覆盖范围

首版提供 QEMU virt 第二个 VirtIO MMIO 插槽的独占绑定，以及块设备接口。
示例实现真正的 VirtIO modern 特性协商、DMA 队列、读写/flush、完成中断和
睡眠唤醒；这些算法位于模块 ELF。示例拒绝只读硬件，只串行提交一个请求。
状态查询包含模块的块传输和中断计数。

最多 8 个常驻模块，每个 ELF 最大 256 KiB，内存映像最大 1 MiB；每次绑定
最多 32 个资源页。重复模块名称、重复绑定、不匹配设备会失败。
当前只有一个可绑定硬件插槽；新总线、其他设备类别、多设备实例和安全卸载
需要扩展相应通用接口。模块是受信任的内核代码，不具备用户进程的故障隔离；
设备错误目前沿用块设备层的内核错误处理方式。

旧的第二块磁盘自动初始化已移除。原先依赖启动时直接访问 source=1 的脚本，
需先显式加载并绑定驱动。
新构建镜像包含管理工具和 `/usr/lib/drivers/vblk.elf` 示例文件，但不会自动加载。
已有持久根盘不会被构建覆盖；需通过原有安装更新流程取得这些新文件，或自行传入。

## 验收

`tests/kmod_hotplug.py` 在私有磁盘副本上启动 QEMU，登录之后才复制驱动源码
及公共 ABI、独立编译新名称 ELF，再在线下载、加载和绑定。
验证绑定前无 vdb、挂载失败，绑定后出现设备并完成用户态 ext3 挂载、读写、
同步、卸载和再次挂载。关闭来宾后用 debugfs 独立检查落盘内容，e2fsck 检查
文件系统；同时验证原始内核、引导 ELF 和测试镜像没有改变且启动次数为一次。
测试还拒绝错误架构、入口、ABI、RWX 段和错误重定位，验证 root 权限、
64 次失败绑定的资源回收、四进程并发读，以及模块执行 supervisor-only CSR
指令。模块不以用户进程出现在 `ps` 中。

2026-11-04：17 项驱动、启动及文件系统回归通过，日志位于
`build/kmod-final-acceptance.log`。随后加入按代数同步和调度恢复同步，最终
内核的 9 项关键复验全部通过（276.91 秒），包括单核/五核驱动热加载、启动、
五核存储/内存/账户流程、单核/五核块能力、五核 ext4 和 FS ELF 热加载；日志
位于 `build/kmod-code-sync-acceptance.log`。来宾详细过程位于
`build/dev/kernel-driver-{1,5}cpu.log`。测试使用私有磁盘，不更新已有持久根盘。
