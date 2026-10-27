# 启动模块 xv.boot

`boot.cppm` 导出两个不返回的入口：`enter(hart_id, device_tree)` 负责
机器态设置，`run()` 负责监管态启动并进入调度器。

启动路径：

```text
entry.S: _entry（设置每核启动栈）
  → entry.cpp: start（C ABI 适配）
  → xv::boot::enter（机器态）
  → mret
  → xv::boot::run（监管态）
  → xv::process::scheduler().run()
```

- `start.cpp`：保存引导核收到的设备树地址，配置特权级、异常/中断委派、
  PMP、计时器与 tp，然后通过 mret 切换到监管态。
- `main.cpp`：引导核依次初始化控制台、打印、内存、进程、陷阱、驱动、
  文件系统缓存和初始进程。其他核等待共享初始化完成后启用本核页表、
  陷阱向量和设备中断。
- `state.cppm`：内部模块分区，只向启动实现提供设备树地址查询。
- `entry.cpp`：只保留汇编需要的 `start` 和 `stack0` C 符号。

共享初始化完成标记使用 release/acquire 原子操作，发布时机在初始进程
创建之后。设备树地址由启动模块持有并显式传入内存服务；旧的无参数
`kinit()` 和依赖全局设备树符号的初始化重载已移除。

临时启动栈仍为每 hart 4 KiB，完成初始化后从内存服务取得默认 2 MiB 的可增长调度栈。
`enter_scheduler_stack` 切换 SP 并尾调用 `scheduler_on_stack`，此后调度与空闲中断使用
新栈；调度栈初始提交 32 KiB，容量与进程内核栈由同一构建参数控制。无全局构造函数。
