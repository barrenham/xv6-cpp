# 开发指南

## 工具链

[environment.yml](../environment.yml) 提供 Clang、clangd、LLD、LLVM 工具、CMake、Ninja 和 Python。QEMU 单独安装，使用 `qemu-system-riscv64`，支持 RISC-V virt、Sstc 和 VirtIO 块设备。

```sh
conda env create -f environment.yml
conda activate OS-LAB
cmake --preset dev
cmake --build --preset dev
```

已有环境通过 `conda env update -n OS-LAB -f environment.yml` 更新。

内核与用户程序以 freestanding C++20 编译，目标为 `riscv64-unknown-elf`、`rv64gc`、`lp64d`，由 LLD 直接链接。内核使用具名模块、虚接口和 RAII，关闭异常和 RTTI。宿主机工具与契约测试使用独立的原生构建。

## 运行与调试

```sh
cmake --build build/dev --target qemu
ctest --preset dev
cmake --build build/dev --target qemu-gdb
```

GDB 加载 `build/dev/kernel.elf`，通过 `target remote :26000` 连接。QEMU 退出组合键为 `Ctrl-a`、`x`。

开发启动使用 `build/dev/fs.img`，磁盘修改保留至镜像重新生成。集成测试使用临时磁盘副本。

通过 `cmake --preset dev -D选项=值` 设置运行参数：

| 选项 | 默认值 | 作用 |
| --- | --- | --- |
| `XV_RAM_MB` | `128` | QEMU 内存容量，单位 MiB |
| `XV_CPUS` | `5` | CPU 数量 |
| `XV_GDB_PORT` | `26000` | GDB 端口 |
| `XV_QEMU` | 自动查找 | QEMU 可执行文件路径 |
| `XV_NET_DHCP` | `ON` | 启动时自动取址 |
| `XV_NET_BACKEND` | `user,id=net0` | QEMU 网络后端 |

链表页分配器使用 `freelist` 预设：`cmake --preset freelist`、`cmake --build --preset freelist`。

## 镜像中的目录

| 路径 | 安装内容 |
| --- | --- |
| `/bin` | sh、文件操作、账户切换和普通网络应用 |
| `/sbin` | init、useradd、netctl |
| `/usr/tests` | 客体测试、cppdemo 和 zombie 演示程序 |
| `/usr/share/doc/xv6/README` | 随镜像附带的 xv6 说明 |
| `/dev`、`/proc`、`/tmp` | 启动挂载的设备、进程信息和 RAM 文件系统 |
| `/etc`、`/home`、`/root` | 账户配置与用户目录 |

根目录只安装目录，不保留程序或 README 副本。默认 PATH 为 `/bin:/sbin:/usr/tests`，因此原有命令和测试名可直接运行；从任意工作目录启动 shell 都使用 `/bin/sh`。

`user/CMakeLists.txt` 按源文件类别确定目标路径；编译产物仍在 `build/dev/user/_名称`。CMake 生成 `build/dev/fs-manifest.tsv`，用 `dir<TAB>八进制权限<TAB>目标路径` 或 `file<TAB>八进制权限<TAB>目标路径<TAB>宿主文件` 明确安装映射，父目录必须先列出。宿主文件路径可以包含空格。

mkfs 拒绝重复目标、非规范路径、超长组件、缺失父目录和超出磁盘格式容量的文件。镜像先写入同目录临时文件，完整写入后原子替换 `fs.img`；失败保留原镜像，正在运行的 QEMU 继续使用原先打开的镜像。重启 QEMU 后加载新布局。

验证安装路径、权限、文件内容和失败回滚：`ctest --test-dir build/dev -R '^image_layout$' --output-on-failure`。

## 编辑器

先完成一次构建，clangd 通过 `.clangd` 读取 `build/dev/compile_commands.json` 和 CMake 生成的模块文件。宿主机工具和契约测试使用 `build/dev/host/compile_commands.json`。

VS Code 配置位于 `.vscode/settings.json`。修改其中的 Conda 路径以匹配本机环境；打开上层工作区时，使用上层工作区的设置。修改 `.cppm` 后重新构建，再执行 **clangd: Restart language server**。

检查跨模块诊断、补全、跳转和悬浮：

```sh
python tests/clangd_modules.py --clangd "$CONDA_PREFIX/bin/clangd"
```

函数说明单独保存在 `docs/functions/`，通过 clangd 外部索引显示。更新流程见 [Doxygen 文档工具](functions/README.md)。

## 代码导航

| 目录 | 内容 |
| --- | --- |
| `kernel/` | 按启动、内存、进程、文件系统、驱动、网络等职责组织的内核模块 |
| `user/apps/` | shell、netctl、nslookup、curl 等应用 |
| `user/runtime/` | 用户运行库、系统调用入口、DNS 与 HTTP 客户端 |
| `user/tests/` | 客体测试程序 |
| `tests/` | 原生契约测试与 QEMU 集成测试 |
| `tools/mkfs/` | 文件系统镜像工具 |
| `cmake/`、`linker/` | 工具链、模块构建与链接布局 |

新增应用在 `user/CMakeLists.txt` 的 `programs` 中登记。内核接口与实现在各自子系统中维护。

模块设计：[启动](../kernel/boot/README.md)、[内存](../kernel/memory/README.md)、[进程](../kernel/process/README.md)、[文件系统](../kernel/fs/README.md)、[驱动](../kernel/drivers/README.md)、[同步](../kernel/sync/README.md)、[陷阱](../kernel/trap/README.md)、[网络](network.md)。

## 登录与测试镜像

首次创建且没有导入密码时，需要设置 root 密码，然后通过 login 进入 shell。构建产物 fs.img 是安装种子，日常启动使用独立的持久 rootfs。重建种子不会修改已有 rootfs；内核更新在重启后生效，用户程序更新需要显式安装或选用新的 rootfs。

构建另外生成 fs-test.img，CTest 使用其独立临时副本，自动走首次密码设置和真实登录流程，不读取交互 QEMU 的账户状态。测试账号密码仅属于临时测试磁盘，不是系统默认密码。测试日志中的 session/workflow 按 CPU 数分别命名。

## 五核与持久化启动

默认 dev 预设和 QEMU 目标使用 XV_CPUS=5，集成测试覆盖 1 核和 5 核，RAM 矩阵串行执行。
`qemu` 和 `qemu-gdb` 通过 `rootfs-image` 目标确保 `data/rootfs.img` 存在，
把它作为第一个 VirtIO 块设备，内核直接以此卷作为 `/`。日常账户与文件不再位于 `/data`。

首次创建 rootfs 时复制最新 `build/dev/fs.img`。新构建目录的 XV_ROOTFS_IMPORT_IMAGE 默认空，
使用者首次启动设置自己的 root 密码。显式 `rootfs-create` 目标要求输出不存在，
不导入旧账户；它不会因为旧 CMake 缓存保留了导入路径而改变这个行为。

需要迁移旧 data/data.img 时，显式配置 XV_ROOTFS_IMPORT_IMAGE，使用 rootfs-image 目标。
工具把其内容导入私有种子副本，保留密码、UID/GID、权限、硬链接及符号链接，
并把 passwd 中旧 `/data/root`、`/data/home/...` 主目录改为 `/root`、`/home/...`。
源镜像须离线且当前用户可读写；工具取得整文件排他锁后只读取内容，不修改源盘。
路径冲突、空间不足或格式错误时不发布新 rootfs，文件和链接中的其他旧绝对路径不自动重写。
已有 rootfs 不会再次导入旧盘。旧构建目录可能缓存原来的导入路径，可设为空以禁用。

可配置 `-DXV_ROOTFS_IMAGE=/absolute/path/rootfs.img` 选择自己的根盘。
已有根盘不随普通 rebuild 被覆盖；退出 QEMU 后，运行 rootfs-update 可将本次构建的
系统程序安装进已有根盘，保留账户和用户目录，并先生成完整备份。
它只更新清单白名单中的程序和文档，不自动删除旧程序、不调整卷容量。
完整命令和保护范围见 [rootfs 工具说明](rootfs-tools.md)。

CTest 始终使用临时私有镜像，不访问真实的 data/rootfs.img 或旧数据盘。
持久化测试覆盖重启、重新生成种子、账户路径、可选辅助盘挂载及旧数据盘迁移。

## 内核栈容量

默认每个进程及每 hart 调度器各保留 2 MiB 虚拟内核栈，初始物理占用 32 KiB，使用时按页扩展。
需要更大容量时可运行：

```sh
cmake --preset dev -DXV_KERNEL_STACK_MIB=4
cmake --build --preset dev
```

支持 2、4、8、16、32、64 MiB；运行时增长到配置容量为止。
该参数调整内核栈，用户程序的用户态栈仍遵循原有布局。

## 虚拟内存回归

新增 vm_buddy_contract、vm_freelist_contract 和 vm_stack_contract。
后者从真实编译数据库生成私有诊断对象和 ELF，正常内核保持 XV_VM_SELF_TEST=0；
诊断模式验证持锁增长、权限错误、应急池耗尽及保护区访问，不改写正常 kernel.elf。
platform-progress 的单核/五核流程还运行新 vmcheck 和更新后的 superpgfork。
协议与边界见 [虚拟内存说明](virtual-memory.md)。
