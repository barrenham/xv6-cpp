# 系统开发文档

开发说明与 `kernel/`、`user/` 代码分离维护。已有源码目录中的历史 README 保留，避免破坏链接。

- [kernel 全量函数导航](functions/kernel/README.md)：259 个文件，1550 个函数/入口条目。
- [Doxygen 原生 hover 的生成、检查和限制](functions/README.md)。
- [等待通道详解](functions/kernel/process/wait-channels.md)。
- [进程睡眠详解](functions/kernel/process/sleep.md)。
- [生命周期详解](functions/kernel/process/lifecycle.md)。
- [公开进程服务详解](functions/kernel/process/service.md)。
- [时钟等待详解](functions/kernel/trap/tick-clock.md)。

- [分层 IPv4、TCP/UDP 与 socket 开发说明](network.md)。

- [统一 File 后端与能力接口](io.md)。

- [网络接口、多网卡选路与 IP-in-IP 隧道](network-interfaces.md)。

- [用户态网络配置与 netctl 管理 ABI](network-management.md)。

- [学习型软件网桥与 bridge 管理命令](network-bridge.md)。
