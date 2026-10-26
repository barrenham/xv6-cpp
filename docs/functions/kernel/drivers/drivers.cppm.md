# `kernel/drivers/drivers.cppm`

开发职责：设备对象、MMIO、设备中断与终端数据流。

源码：[打开实际文件](../../../../kernel/drivers/drivers.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.drivers`。

直接模块依赖：`:device`, `:serial`, `:character`, `:block`, `:interrupt`, `:router`, `:ldisc`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::drivers::block_device`](board/qemu_virt.cpp.md#fn-72e31996ffc2b022)
- [`xv::drivers::console`](board/qemu_virt.cpp.md#fn-cb18263197838102)
- [`xv::drivers::create_pty`](tty/pty/pty.cpp.md#fn-8801034b0305377a)
- [`xv::drivers::handle_external_interrupt`](board/qemu_virt.cpp.md#fn-8819b4e9b6bdacb6)
- [`xv::drivers::initialize_console`](board/qemu_virt.cpp.md#fn-cacccb8b1ed2107b)
- [`xv::drivers::initialize_hart`](board/qemu_virt.cpp.md#fn-bea9233c904d33be)
- [`xv::drivers::initialize_interrupts`](board/qemu_virt.cpp.md#fn-ea116721d09e5c14)
- [`xv::drivers::initialize_ptys`](tty/pty/pty.cpp.md#fn-0c7545d488e75354)
- [`xv::drivers::interrupt_controller`](board/qemu_virt.cpp.md#fn-276393f76c615b60)
- [`xv::drivers::pty_timer_tick`](tty/pty/pty.cpp.md#fn-b050f1554cf27075)
- [`xv::drivers::serial_port`](board/qemu_virt.cpp.md#fn-ee46a944352f0df2)
- [`xv::drivers::terminal_timer_tick`](board/qemu_virt.cpp.md#fn-a345578c92b3adb7)

