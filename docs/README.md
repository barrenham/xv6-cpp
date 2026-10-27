# 系统开发文档

开发说明与 `kernel/`、`user/` 代码分离维护。已有源码目录中的历史 README 保留，避免破坏链接。

- [开发指南](development.md)：构建、运行、调试与编辑器配置。
- [kernel 全量函数导航](functions/kernel/README.md)：280 个文件，1829 个函数/入口条目。
- [Doxygen 原生 hover 的生成、检查和限制](functions/README.md)。
- [等待通道详解](functions/kernel/process/wait-channels.md)。
- [进程睡眠详解](functions/kernel/process/sleep.md)。
- [生命周期详解](functions/kernel/process/lifecycle.md)。
- [公开进程服务详解](functions/kernel/process/service.md)。
- [时钟等待详解](functions/kernel/trap/tick-clock.md)。

- [分层 IPv4、TCP/UDP 与 socket 开发说明](network.md)。

- [统一 File 后端与能力接口](io.md)。

- [VFS 与 devfs](devfs.md)：设备目录、挂载路径、字符设备与生命周期。

- [命名 PTY](named-pty.md)：ptmx、pts、设备会话及编号生命周期。

- [procfs 与 ps/free](procfs.md)：动态快照、描述符观测、内存统计与用户态工具。

- [RAM 文件系统](ramfs.md)：/tmp、按页存储、目录与文件生命周期。

- [网络接口、多网卡选路与 IP-in-IP 隧道](network-interfaces.md)。

- [用户态网络配置与 netctl 管理 ABI](network-management.md)。

- [学习型软件网桥与 bridge 管理命令](network-bridge.md)。

- [DHCP 自动取址、续租与配置归属](network-dhcp.md)。

- [DNS 用户态解析与 nslookup](network-dns.md)：DHCP DNS、A/CNAME、UDP/TCP、运行库 API 与测试。

- [HTTP 客户端与 curl](network-http.md)：流式下载、重定向、HTTP 定界、用户态类库与测试。

- [TCP 滑动窗口与连接管理](network-tcp.md)：收发队列、乱序重排、NewReno、重传与关闭。

- [环境变量与 shell](environment.md)：环境继承、execve、PATH、引号和内建命令。

- [用户、组与权限](users-permissions.md)：凭据、账户工具、文件权限、授权边界与磁盘格式。

- [shell 输入编辑与补全](shell-editing.md)。
- [pwd、cp、mv 与原子重命名](file-tools.md)。
- [动态 RAM 挂载与生命周期](mounting.md)。
