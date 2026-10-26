# `kernel/syscall/handlers.cppm`

开发职责：用户参数校验、描述符取得及系统调用返回 ABI。

源码：[打开实际文件](../../../../kernel/syscall/handlers.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.syscall:handlers`。

## 函数导航

- [`xv::syscall::sys_accept`](syssocket.cpp.md#fn-a551ee0c6cc31b66)
- [`xv::syscall::sys_bind`](handlers.cppm.md#fn-7add8f742a0f380b)
- [`xv::syscall::sys_chdir`](sysfile.cpp.md#fn-5db6c22d9b99841b)
- [`xv::syscall::sys_close`](sysfile.cpp.md#fn-4692a7b4aafb1a42)
- [`xv::syscall::sys_connect`](syssocket.cpp.md#fn-6cc9bb91284d2288)
- [`xv::syscall::sys_cpupin`](handlers.cppm.md#fn-e260733f13c74103)
- [`xv::syscall::sys_dup`](sysfile.cpp.md#fn-d76ff7559f643ffa)
- [`xv::syscall::sys_exec`](sysfile.cpp.md#fn-d063aa5f0fd12e38)
- [`xv::syscall::sys_exit`](sysproc.cpp.md#fn-407b853e794ab99c)
- [`xv::syscall::sys_fcntl`](sysfile.cpp.md#fn-2940e0efab81491b)
- [`xv::syscall::sys_fork`](sysproc.cpp.md#fn-c9d853f3df63566a)
- [`xv::syscall::sys_fstat`](sysfile.cpp.md#fn-c2ca80b2260b826c)
- [`xv::syscall::sys_getpeername`](syssocket.cpp.md#fn-ad79d7621aebac76)
- [`xv::syscall::sys_getpgrp`](sysproc.cpp.md#fn-2edb2a42a8a878d9)
- [`xv::syscall::sys_getpid`](sysproc.cpp.md#fn-77147847a5efd0c1)
- [`xv::syscall::sys_getsid`](sysproc.cpp.md#fn-5321341cbd693195)
- [`xv::syscall::sys_getsockname`](syssocket.cpp.md#fn-34ce2f3f218a1bcb)
- [`xv::syscall::sys_getsockopt`](syssocket.cpp.md#fn-dfca0367665d52c2)
- [`xv::syscall::sys_ioctl`](sysfile.cpp.md#fn-868f8327fa5e0719)
- [`xv::syscall::sys_kill`](sysproc.cpp.md#fn-323c866c33cd7f90)
- [`xv::syscall::sys_ksupernpte`](sysvm.cpp.md#fn-854777261a9f0a46)
- [`xv::syscall::sys_link`](sysfile.cpp.md#fn-f0e4c3a2b61b65f2)
- [`xv::syscall::sys_listen`](syssocket.cpp.md#fn-d8dff478749541a9)
- [`xv::syscall::sys_mkdir`](sysfile.cpp.md#fn-697c039ba51257c6)
- [`xv::syscall::sys_mknod`](sysfile.cpp.md#fn-a335916d7a484f15)
- [`xv::syscall::sys_netctl`](sysnetctl.cpp.md#fn-470182ff56d32e47)
- [`xv::syscall::sys_open`](sysfile.cpp.md#fn-db9a14b0a5f34f13)
- [`xv::syscall::sys_openpty`](sysfile.cpp.md#fn-6f37e9433a6bd907)
- [`xv::syscall::sys_pause`](sysproc.cpp.md#fn-2e8395597b7be064)
- [`xv::syscall::sys_pgaccess`](sysproc.cpp.md#fn-d0463f44687962b6)
- [`xv::syscall::sys_pgpte`](sysvm.cpp.md#fn-ca4e57de1ea0f712)
- [`xv::syscall::sys_pipe`](sysfile.cpp.md#fn-4fdcb85095f658e9)
- [`xv::syscall::sys_poll`](syspoll.cpp.md#fn-9423b8d80c87e07b)
- [`xv::syscall::sys_read`](sysfile.cpp.md#fn-fe1cdff590d53ee1)
- [`xv::syscall::sys_recv`](handlers.cppm.md#fn-0e996b82215416d8)
- [`xv::syscall::sys_recvfrom`](syssocket.cpp.md#fn-f11541d42bf45977)
- [`xv::syscall::sys_rwlktest`](handlers.cppm.md#fn-1d55124bdf88bbe8)
- [`xv::syscall::sys_sbrk`](sysproc.cpp.md#fn-e24ad0a860144ff2)
- [`xv::syscall::sys_send`](handlers.cppm.md#fn-2528203d05b39b5f)
- [`xv::syscall::sys_sendto`](syssocket.cpp.md#fn-4280b55882ad0bde)
- [`xv::syscall::sys_setpgid`](sysproc.cpp.md#fn-9ebd01f65216ef04)
- [`xv::syscall::sys_setsid`](sysproc.cpp.md#fn-cd3291a03e4e4fd4)
- [`xv::syscall::sys_shutdown`](syssocket.cpp.md#fn-936eb95148b68d1d)
- [`xv::syscall::sys_sigaction`](sysproc.cpp.md#fn-661c45c2c27dca21)
- [`xv::syscall::sys_sigdisposition`](sysproc.cpp.md#fn-fe4a961d03b34ad1)
- [`xv::syscall::sys_sigpending`](sysproc.cpp.md#fn-65a6c1417c187dda)
- [`xv::syscall::sys_sigprocmask`](sysproc.cpp.md#fn-950b274504f46ead)
- [`xv::syscall::sys_sigreturn`](sysproc.cpp.md#fn-1964b15046da773a)
- [`xv::syscall::sys_sigsend`](sysproc.cpp.md#fn-166824fa4bf09569)
- [`xv::syscall::sys_sigsuspend`](sysproc.cpp.md#fn-c136e1cfe8a4c0d7)
- [`xv::syscall::sys_sockbind`](syssocket.cpp.md#fn-08947764259f8414)
- [`xv::syscall::sys_socket`](syssocket.cpp.md#fn-1248323fc95ac4c4)
- [`xv::syscall::sys_sync`](sysfile.cpp.md#fn-f630890d41c7271a)
- [`xv::syscall::sys_unbind`](handlers.cppm.md#fn-fdefa0ba0439a4a9)
- [`xv::syscall::sys_unlink`](sysfile.cpp.md#fn-4aa7dc56022e07b7)
- [`xv::syscall::sys_uptime`](sysproc.cpp.md#fn-d5a85217c5ec8031)
- [`xv::syscall::sys_vmprint`](sysproc.cpp.md#fn-b43c13f418afd200)
- [`xv::syscall::sys_wait`](sysproc.cpp.md#fn-0e651ab6af812b15)
- [`xv::syscall::sys_waitpid`](sysproc.cpp.md#fn-47570f7534c0492e)
- [`xv::syscall::sys_write`](sysfile.cpp.md#fn-dc225934cc4ef27f)

<a id="fn-7add8f742a0f380b"></a>

## `xv::syscall::sys_bind`

```doxygen
@fn unsigned long xv::syscall::sys_bind(void)
@brief 网络系统调用：校验端口 1..65535 后绑定当前 PID；返回 0 或 -1。
@details 调用 xv.net 服务；当前 dev 配置已启用 LAB_NET。
```

<a id="fn-fdefa0ba0439a4a9"></a>

## `xv::syscall::sys_unbind`

```doxygen
@fn unsigned long xv::syscall::sys_unbind(void)
@brief 网络系统调用：校验端口后释放当前 PID 的绑定引用；最后一个引用释放排队页。
@details 调用 xv.net 服务；当前 dev 配置已启用 LAB_NET。
```

<a id="fn-2528203d05b39b5f"></a>

## `xv::syscall::sys_send`

```doxygen
@fn unsigned long xv::syscall::sys_send(void)
@brief 网络系统调用：读取源端口、目标 IPv4/端口、用户指针和长度，拒绝负长度及超范围端口，再交给网络栈组包；可返回 -EAGAIN。
@details 调用 xv.net 服务；当前 dev 配置已启用 LAB_NET。
```

<a id="fn-0e996b82215416d8"></a>

## `xv::syscall::sys_recv`

```doxygen
@fn unsigned long xv::syscall::sys_recv(void)
@brief 网络系统调用：读取接收元信息与数据缓冲区的用户地址，拒绝负容量或超范围端口；阻塞接收支持信号中断和 SA_RESTART。
@details 调用 xv.net 服务；当前 dev 配置已启用 LAB_NET。
```

<a id="fn-1d55124bdf88bbe8"></a>

## `xv::syscall::sys_rwlktest`

```doxygen
@fn unsigned long xv::syscall::sys_rwlktest(void)
@brief 系统调用入口：rwlktest；从用户参数转入内核服务并返回 ABI 结果。
@return 返回类型为 `unsigned long ()`；本条目没有可独立分析的普通函数体，具体返回约定见对应实现及调用方。
@note 配置范围：LAB_LOCK/LAB_NET。此条目只覆盖独立文档，不向默认 dev 的原生索引发布未经该配置完整构建验证的符号。
@note 系统调用处理函数使用内核 64 位返回通道；负错误值经无符号类型传回时保持位模式，由用户 ABI 按相应整型解释。
```

<a id="fn-e260733f13c74103"></a>

## `xv::syscall::sys_cpupin`

```doxygen
@fn unsigned long xv::syscall::sys_cpupin(void)
@brief 系统调用入口：cpupin；从用户参数转入内核服务并返回 ABI 结果。
@return 返回类型为 `unsigned long ()`；本条目没有可独立分析的普通函数体，具体返回约定见对应实现及调用方。
@note 配置范围：LAB_LOCK/LAB_NET。此条目只覆盖独立文档，不向默认 dev 的原生索引发布未经该配置完整构建验证的符号。
@note 系统调用处理函数使用内核 64 位返回通道；负错误值经无符号类型传回时保持位模式，由用户 ABI 按相应整型解释。
```
