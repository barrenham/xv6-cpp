# xv-labs-cpp-2026

当前版本：`v0.0.2`。

用 C++20 重构的 xv6 RISC-V 教学操作系统。内核按具名模块组织，通过类接口和 RAII 管理设备、内存、文件与进程资源。使用 CMake、Ninja 和 Clang/LLD 构建，在 QEMU 上运行。

## 功能

- **内存**：设备树 RAM 发现、伙伴/链表页分配器、Sv39 页表与大页映射、虚拟区域登记、按需内核栈及应急补页、写时复制 fork。
- **进程**：多核调度、动态进程登记、信号处理、进程组、shell 作业控制、历史编辑/补全、脚本与环境变量。
- **文件与终端**：日志文件系统、VFS、devfs/ramfs、动态 RAM/磁盘挂载、持久 rootfs、符号链接、pwd/cp/mv、随机文件 I/O、统一 I/O 后端、管道、TTY/命名 PTY、procfs、ps/free、poll 与非阻塞 I/O。
- **用户与权限**：UID/GID、附加组、文件权限、umask、登录与密码、用户/组管理、普通用户改密、chmod/chown。
- **网络**：E1000、IPv4、ARP、ICMP、TCP/UDP、socket、多网卡、网桥、IP-in-IP 隧道与 IPv4 报文过滤。
- **网络应用**：DHCP 自动取址、DNS 解析、netctl、nslookup、iptables 和 HTTP curl。

## 构建与运行

首次使用时安装 Conda 和 `qemu-system-riscv64`，在项目根目录执行：

```sh
conda env create -f environment.yml
conda activate OS-LAB
cmake --preset dev
cmake --build build/dev --target rootfs-create
cmake --build build/dev --target qemu
```

已有自己的根盘时无需再次运行 `rootfs-create`，直接使用 `qemu` 目标启动。

退出 QEMU：先按 `Ctrl-a`，再按 `x`。运行测试：

```sh
ctest --preset dev
```

默认启动 5 核，使用 `data/rootfs.img` 作为挂载到 `/` 的持久根文件系统。首次创建以干净的构建镜像为种子，不默认导入旧账户；使用者在首次启动时设置自己的 root 密码，没有默认密码。登录进入 `/root`，账户位于 `/etc`，普通用户主目录位于 `/home`。后续构建不会覆盖 rootfs；内核更新直接生效，更新已安装的用户程序时，退出 QEMU 后运行 `cmake --build build/dev --target rootfs-update`，会先备份并保留账户和用户目录。系统通过 DHCP 获取地址，客体 shell 中可以运行：

```sh
netctl dhcp wait eth0
nslookup example.com
curl http://example.com/
```

要生成另一份独立 rootfs，可配置 `-DXV_ROOTFS_IMAGE="$PWD/data/my-rootfs.img"` 后再次运行 `rootfs-create`。该目标要求输出尚不存在，不会覆盖已有磁盘。

## 文档

- [v0.0.2 版本说明](docs/releases/v0.0.2.md)：rootfs 工具、防火墙与网络层重构。

- [rootfs 创建与程序更新](docs/rootfs-tools.md)：新使用者设置自己的密码，离线更新现有系统程序。
- [开发指南](docs/development.md)：构建选项、调试和 clangd 配置。
- [系统开发文档](docs/README.md)：模块设计、网络协议与用户态接口。
- [内核函数导航](docs/functions/kernel/README.md)：独立 Doxygen 函数说明。

## 来源与许可

基于 xv6-labs-2026，采用 [MIT 许可](LICENSE)。上游说明见 [README.xv6](README.xv6)。

IPv4 报文过滤与 `iptables` 命令的分层设计、用法及支持范围见 [防火墙说明](docs/network-firewall.md)。
