# 系统关机与重启

更新开发根盘并重新启动 QEMU 后，以 root 执行：

```sh
poweroff   # 关机并退出 QEMU
reboot     # 重启客户机，QEMU 进程继续运行
```

命令从串口或 fbterm 执行均可。`poweroff` 会关闭整个客户机并退出 QEMU；`reboot` 会复位客户机，重新引导内核、驱动和 FS 服务，随后回到登录界面。无需切换回宿主机按终端快捷键。两条命令的 `--help` 显示用法，其他参数被拒绝。

## 执行过程

`/bin/poweroff` 和 `/bin/reboot` 分别调用 `SYS_poweroff` 和 `SYS_reboot` 系统调用，共用权限检查及同步流程。内核检查有效 UID 为 0；非 root 返回 EPERM。运行库按现有 POSIX 接口约定返回 -1 并设置 errno。

内核先确认存在受支持的电源控制设备，再同步根文件系统和所有已挂载的文件系统。同步沿用 VFS 的服务端同步接口，包含映射脏页写回和后端日志提交。此时保留调度和中断，使用户态 FS 线程能响应请求。任何同步失败都会中止关机并返回 EIO，不提供跳过同步的强制参数。

同步成功后输出完成消息，关闭当前 hart 的中断，执行 I/O 屏障并向平台关机设备写入关机请求。QEMU RISC-V virt 使用 SiFive test finisher；地址从 DT 中的 `sifive,test1` / `sifive,test0` 节点发现并加入内核 MMIO 映射，不在系统调用中硬编码物理地址。写入 `0x5555` 请求成功关机，写入 `0x7777` 请求整机复位，对照 [QEMU finisher 实现](https://github.com/qemu/qemu/blob/v9.2.0/hw/misc/sifive_test.c)。没有这个控制器时返回 ENODEV，移植到真实硬件需要实现对应的平台关机操作。

这是基础的同步后关机与重启功能，尚未实现服务管理器逐个停止服务、停止全部后台写入任务、定时关机。请先保存编辑器里的内容，并结束正在写入重要数据的任务；关机不会保存应用程序尚未写出的用户态缓冲。

## 验证

`poweroff_1cpu`、`poweroff_5cpu` 在独立磁盘上验证帮助、非法参数、非 root 系统调用拒绝、root 关机、QEMU 成功退出状态，以及不手动执行 sync 的文件持久化。退出后运行只读 e2fsck 检查磁盘，并直接读取测试文件核对内容。设备树原生测试检查 finisher 的发现及映射范围。

2026-11-08 验证通过：`poweroff_1cpu`、`poweroff_5cpu` 和 `device_tree_contract`，共 3 项。日志见 `build/poweroff-validation.log` 和 `build/dev/poweroff-{1,5}cpu.log`。

`reboot_1cpu`、`reboot_5cpu` 在同一个 QEMU 进程中连续重启两次，验证权限、重新登录、持久化文件和账户；五核用例还验证重新获取 DHCP 地址及 ping，最后通过 poweroff 正常关机并执行只读 e2fsck。

2026-11-08 重启验证通过：`reboot_1cpu`、`reboot_5cpu`，并回归 `poweroff_1cpu`、`poweroff_5cpu`，共 4 项。日志见 `build/reboot-validation.log`、`build/dev/reboot-{1,5}cpu.log`。
