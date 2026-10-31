# 系统开发文档

开发说明与 `kernel/`、`user/` 代码分离维护。已有源码目录中的历史 README 保留，避免破坏链接。

- [虚拟区域、缺页与 COW](virtual-memory.md)：区域归属、权限分类、持锁补页和 fork 共享。
- [VirtIO-GPU 2D 显示与 /dev/fb0](graphics.md)。
- [分辨率配置](display-resolution.md)：默认1280×800，最高1920×1080，终端网格自适应。
- [终端字形、滚屏与跨行编辑](terminal-rendering.md)。
- [图形终端性能排查](display-performance.md)：分阶段计时、串口背压与VNC对照。
- [串口镜像与批量输出优化](terminal-output-optimization.md)：批量短写、镜像开关和前后对照。
- [嵌套PTY的换行和逐层退出](nested-pty.md)：修正提示符偏移及Ctrl-]穿透。
- [fbterm 用户态图形终端](fbterm.md)：自适应字符网格、滚屏、PTY shell 与 VNC/串口输入。
- [VirtIO 键盘](keyboard.md)：VNC 直接输入、事件 ABI、队列及验证。
- [ext3 模块、格式选项及离线转换](ext3.md)，[本轮验收记录](ext3-dev-proc-acceptance.json)。
- [目录枚举接口](directories.md)：getdents64 完整名称与兼容 read ABI。
- [rootfs 创建与程序更新](rootfs-tools.md)：独立新建、原子升级、备份与账户保留。
- [开发指南](development.md)：构建、运行、调试与编辑器配置。
- [kernel 全量函数导航](functions/kernel/README.md)：404 个文件，5230 个函数/入口条目。
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

- [IPv4 防火墙与 iptables](network-firewall.md)：分层设计、规则管理、收发过滤与扩展边界。

- [用户态网络配置与 netctl 管理 ABI](network-management.md)。

- [学习型软件网桥与 bridge 管理命令](network-bridge.md)。

- [DHCP 自动取址、续租与配置归属](network-dhcp.md)。

- [DNS 用户态解析与 nslookup](network-dns.md)：DHCP DNS、A/CNAME、UDP/TCP、运行库 API 与测试。

- [ping 与 clear](network-tools.md)：ICMP 测量、受限数据报 socket 和终端清屏。
- [HTTP 客户端与 curl](network-http.md)：流式下载、重定向、HTTP 定界、用户态类库与测试。

- [TCP 滑动窗口与连接管理](network-tcp.md)：收发队列、乱序重排、NewReno、重传与关闭。

- [环境变量与 shell](environment.md)：环境继承、execve、PATH、引号和内建命令。

- [用户、组与权限](users-permissions.md)：凭据、账户工具、文件权限、授权边界与磁盘格式。

- [shell 输入编辑与补全](shell-editing.md)。
- [pwd、cp、mv 与原子重命名](file-tools.md)。
- [动态 RAM 挂载与生命周期](mounting.md)。

- [符号链接与路径展开](symbolic-links.md)。

- [PID 索引与子进程事件等待](process-index-wait.md)：PID 红黑树、事件队列、生命周期与多核锁协议。

- [fork 成本调研](fork-cost-investigation.md)：原版工作量对照、分阶段成本及内存填充实验。
