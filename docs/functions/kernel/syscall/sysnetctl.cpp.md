# `kernel/syscall/sysnetctl.cpp`

## 函数导航

- [`xv::syscall::sys_netctl`](sysnetctl.cpp.md#fn-470182ff56d32e47)

<a id="fn-470182ff56d32e47"></a>

## `xv::syscall::sys_netctl`

```doxygen
@fn unsigned long xv::syscall::sys_netctl()
@brief 处理网络查询与管理员配置请求。
@details 先识别操作类别，接口/地址/路由/邻居/网桥及 DHCP 状态查询可读；修改配置要求有效 UID 为 0。仍验证请求结构及具体管理器约束，不信任用户缓冲。
@details 64 至 73 为防火墙专属操作范围，复制独立的 128 字节 firewall_request；仅链与规则查询允许普通用户，全部修改要求有效 UID 为 0。用户复制始终位于 Firewall 锁外。
```
