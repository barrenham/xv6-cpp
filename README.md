# xv-labs-cpp-2026

用 C++20 重构的 xv6 RISC-V 教学操作系统。内核按具名模块组织，通过类接口和 RAII 管理设备、内存、文件与进程资源。使用 CMake、Ninja 和 Clang/LLD 构建，在 QEMU 上运行。

## 功能

- **内存**：设备树 RAM 发现、伙伴/链表页分配器、Sv39 页表与大页映射。
- **进程**：多核调度、动态进程登记、信号处理、进程组、shell 作业控制与环境变量。
- **文件与终端**：日志文件系统、VFS、devfs/ramfs、统一 I/O 后端、管道、TTY/命名 PTY、procfs、ps/free、poll 与非阻塞 I/O。
- **用户与权限**：UID/GID、附加组、文件权限、umask、useradd/su、chmod/chown。
- **网络**：E1000、IPv4、ARP、ICMP、TCP/UDP、socket、多网卡、网桥与 IP-in-IP 隧道。
- **网络应用**：DHCP 自动取址、DNS 解析、netctl、nslookup 和 HTTP curl。

## 构建与运行

安装 Conda 和 `qemu-system-riscv64`，在项目根目录执行：

```sh
conda env create -f environment.yml
conda activate OS-LAB
cmake --preset dev
cmake --build --preset dev
cmake --build build/dev --target qemu
```

退出 QEMU：先按 `Ctrl-a`，再按 `x`。运行测试：

```sh
ctest --preset dev
```

系统启动后通过 DHCP 获取地址。客体 shell 中可以运行：

```sh
netctl dhcp wait eth0
nslookup example.com
curl http://example.com/
```

## 文档

- [开发指南](docs/development.md)：构建选项、调试和 clangd 配置。
- [系统开发文档](docs/README.md)：模块设计、网络协议与用户态接口。
- [内核函数导航](docs/functions/kernel/README.md)：独立 Doxygen 函数说明。

## 来源与许可

基于 xv6-labs-2026，采用 [MIT 许可](LICENSE)。上游说明见 [README.xv6](README.xv6)。
