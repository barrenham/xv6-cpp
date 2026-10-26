# `kernel/syscall/sysnetctl.cpp`

## 函数导航

- [`xv::syscall::sys_netctl`](sysnetctl.cpp.md#fn-470182ff56d32e47)

<a id="fn-470182ff56d32e47"></a>

## `xv::syscall::sys_netctl`

```doxygen
@fn unsigned long xv::syscall::sys_netctl()
@brief 先完整复制并验证用户管理请求，交网络服务后仅将成功查询的零填充快照复制回用户；复制不持网络锁，非法指针返回 -EFAULT。
@return 沿用管理服务的 1/0/负 errno；任一用户复制失败为 -EFAULT。
@note 修改操作只读请求，不在修改完成后尝试复制回用户。查询失败也不写出。
```
