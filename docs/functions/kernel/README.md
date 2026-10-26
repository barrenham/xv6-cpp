# kernel 全量函数开发文档

本文是按实际源码目录建立的导航，函数说明与运行代码分离。每个文件页提供声明/定义到唯一函数条目的链接。

| 子系统 | 文件数 | 函数/入口条目数 |
| --- | ---: | ---: |
| [arch](#arch) | 4 | 49 |
| [boot](#boot) | 6 | 8 |
| [drivers](#drivers) | 33 | 204 |
| [fs](#fs) | 26 | 199 |
| [memory](#memory) | 22 | 173 |
| [net](#net) | 4 | 29 |
| [process](#process) | 58 | 323 |
| [support](#support) | 10 | 32 |
| [sync](#sync) | 12 | 47 |
| [syscall](#syscall) | 11 | 60 |
| [trap](#trap) | 20 | 89 |
| [tty](#tty) | 1 | 0 |

范围：207 个 C/C++/共享 include/汇编文件；默认配置 1148 个唯一 C++ 函数项，连同条件接口、共享实现和汇编入口共 1213 条文档。

逐函数说明包含接口签名、参数、可见状态变化、调用/构造表达式、分支条件及返回表达式；锁、睡眠、用户地址边界和原子操作另行标明。等待/调度、信号帧、管道/TTY/UART、页表复制、日志提交等关键路径额外提供语义说明。源码结构记录不等于所有条目都经过逐行人工审查，遇到细节仍应结合源文件与测试核对。

<a id="arch"></a>

## arch

- [`kernel/arch/arch.cppm`](arch/arch.cppm.md)
- [`kernel/arch/riscv/register_constants.h`](arch/riscv/register_constants.h.md)
- [`kernel/arch/riscv/register_operations.inc`](arch/riscv/register_operations.inc.md)
- [`kernel/arch/riscv/riscv.h`](arch/riscv/riscv.h.md)


<a id="boot"></a>

## boot

- [`kernel/boot/boot.cppm`](boot/boot.cppm.md)
- [`kernel/boot/entry.S`](boot/entry.S.md)
- [`kernel/boot/entry.cpp`](boot/entry.cpp.md)
- [`kernel/boot/main.cpp`](boot/main.cpp.md)
- [`kernel/boot/start.cpp`](boot/start.cpp.md)
- [`kernel/boot/state.cppm`](boot/state.cppm.md)


<a id="drivers"></a>

## drivers

- [`kernel/drivers/block/block_device.cppm`](drivers/block/block_device.cppm.md)
- [`kernel/drivers/block/virtio/protocol.cppm`](drivers/block/virtio/protocol.cppm.md)
- [`kernel/drivers/block/virtio/virtio_block.cpp`](drivers/block/virtio/virtio_block.cpp.md)
- [`kernel/drivers/block/virtio/virtio_block.cppm`](drivers/block/virtio/virtio_block.cppm.md)
- [`kernel/drivers/board/qemu_virt.cpp`](drivers/board/qemu_virt.cpp.md)
- [`kernel/drivers/console/character_device.cppm`](drivers/console/character_device.cppm.md)
- [`kernel/drivers/device.cppm`](drivers/device.cppm.md)
- [`kernel/drivers/device_numbers.h`](drivers/device_numbers.h.md)
- [`kernel/drivers/drivers.cppm`](drivers/drivers.cppm.md)
- [`kernel/drivers/interrupt/interrupt_controller.cppm`](drivers/interrupt/interrupt_controller.cppm.md)
- [`kernel/drivers/interrupt/interrupt_router.cpp`](drivers/interrupt/interrupt_router.cpp.md)
- [`kernel/drivers/interrupt/interrupt_router.cppm`](drivers/interrupt/interrupt_router.cppm.md)
- [`kernel/drivers/interrupt/plic.cpp`](drivers/interrupt/plic.cpp.md)
- [`kernel/drivers/interrupt/plic.cppm`](drivers/interrupt/plic.cppm.md)
- [`kernel/drivers/mmio.cppm`](drivers/mmio.cppm.md)
- [`kernel/drivers/platform/kernel_platform.cpp`](drivers/platform/kernel_platform.cpp.md)
- [`kernel/drivers/platform/platform.cppm`](drivers/platform/platform.cppm.md)
- [`kernel/drivers/serial/serial_port.cppm`](drivers/serial/serial_port.cppm.md)
- [`kernel/drivers/serial/uart16550.cpp`](drivers/serial/uart16550.cpp.md)
- [`kernel/drivers/serial/uart16550.cppm`](drivers/serial/uart16550.cppm.md)
- [`kernel/drivers/tty/core/terminal_output.cppm`](drivers/tty/core/terminal_output.cppm.md)
- [`kernel/drivers/tty/core/tty.cpp`](drivers/tty/core/tty.cpp.md)
- [`kernel/drivers/tty/core/tty.cppm`](drivers/tty/core/tty.cppm.md)
- [`kernel/drivers/tty/ldisc/line_discipline.cppm`](drivers/tty/ldisc/line_discipline.cppm.md)
- [`kernel/drivers/tty/ldisc/n_tty.cpp`](drivers/tty/ldisc/n_tty.cpp.md)
- [`kernel/drivers/tty/ldisc/n_tty.cppm`](drivers/tty/ldisc/n_tty.cppm.md)
- [`kernel/drivers/tty/pty/pty.cpp`](drivers/tty/pty/pty.cpp.md)
- [`kernel/drivers/tty/pty/pty.cppm`](drivers/tty/pty/pty.cppm.md)

- [`kernel/drivers/net/e1000/e1000.cpp`](drivers/net/e1000/e1000.cpp.md)
- [`kernel/drivers/net/e1000/e1000.cppm`](drivers/net/e1000/e1000.cppm.md)
- [`kernel/drivers/net/ethernet.cppm`](drivers/net/ethernet.cppm.md)
- [`kernel/drivers/pci/qemu_pci.cpp`](drivers/pci/qemu_pci.cpp.md)
- [`kernel/drivers/pci/qemu_pci.cppm`](drivers/pci/qemu_pci.cppm.md)

<a id="fs"></a>

## fs

- [`kernel/fs/cache/block_cache.cppm`](fs/cache/block_cache.cppm.md)
- [`kernel/fs/cache/lru_block_cache.cpp`](fs/cache/lru_block_cache.cpp.md)
- [`kernel/fs/cache/lru_block_cache.cppm`](fs/cache/lru_block_cache.cppm.md)
- [`kernel/fs/fcntl.h`](fs/fcntl.h.md)
- [`kernel/fs/file/file_operations.cpp`](fs/file/file_operations.cpp.md)
- [`kernel/fs/file/file_operations.cppm`](fs/file/file_operations.cppm.md)
- [`kernel/fs/file/open_file.cpp`](fs/file/open_file.cpp.md)
- [`kernel/fs/file/open_file.cppm`](fs/file/open_file.cppm.md)
- [`kernel/fs/format/layout.h`](fs/format/layout.h.md)
- [`kernel/fs/fs.cppm`](fs/fs.cppm.md)
- [`kernel/fs/inode/filesystem.cppm`](fs/inode/filesystem.cppm.md)
- [`kernel/fs/inode/inode.cppm`](fs/inode/inode.cppm.md)
- [`kernel/fs/inode/xv6_filesystem.cpp`](fs/inode/xv6_filesystem.cpp.md)
- [`kernel/fs/inode/xv6_filesystem.cppm`](fs/inode/xv6_filesystem.cppm.md)
- [`kernel/fs/io_status.h`](fs/io_status.h.md)
- [`kernel/fs/journal/journal.cpp`](fs/journal/journal.cpp.md)
- [`kernel/fs/journal/journal.cppm`](fs/journal/journal.cppm.md)
- [`kernel/fs/path/path_operations.cpp`](fs/path/path_operations.cpp.md)
- [`kernel/fs/pipe/pipe.cpp`](fs/pipe/pipe.cpp.md)
- [`kernel/fs/pipe/pipe.cppm`](fs/pipe/pipe.cppm.md)
- [`kernel/fs/pipe/pipe_factory.cpp`](fs/pipe/pipe_factory.cpp.md)
- [`kernel/fs/platform/kernel_platform.cpp`](fs/platform/kernel_platform.cpp.md)
- [`kernel/fs/platform/platform.cppm`](fs/platform/platform.cppm.md)
- [`kernel/fs/poll.h`](fs/poll.h.md)
- [`kernel/fs/service.cpp`](fs/service.cpp.md)
- [`kernel/fs/stat.h`](fs/stat.h.md)


<a id="memory"></a>

## memory

- [`kernel/memory/address/physical_address.cppm`](memory/address/physical_address.cppm.md)
- [`kernel/memory/address/physical_memory_range.cppm`](memory/address/physical_memory_range.cppm.md)
- [`kernel/memory/allocator/allocator_detail.cppm`](memory/allocator/allocator_detail.cppm.md)
- [`kernel/memory/allocator/allocator_service.cpp`](memory/allocator/allocator_service.cpp.md)
- [`kernel/memory/allocator/buddy/buddy_page_allocator.cpp`](memory/allocator/buddy/buddy_page_allocator.cpp.md)
- [`kernel/memory/allocator/buddy/buddy_page_allocator.cppm`](memory/allocator/buddy/buddy_page_allocator.cppm.md)
- [`kernel/memory/allocator/freelist/freelist_page_allocator.cpp`](memory/allocator/freelist/freelist_page_allocator.cpp.md)
- [`kernel/memory/allocator/freelist/freelist_page_allocator.cppm`](memory/allocator/freelist/freelist_page_allocator.cppm.md)
- [`kernel/memory/allocator/page_allocator.cppm`](memory/allocator/page_allocator.cppm.md)
- [`kernel/memory/boot/boot_memory.cpp`](memory/boot/boot_memory.cpp.md)
- [`kernel/memory/boot/boot_memory.cppm`](memory/boot/boot_memory.cppm.md)
- [`kernel/memory/boot/kalloc.cpp`](memory/boot/kalloc.cpp.md)
- [`kernel/memory/layout/memlayout.h`](memory/layout/memlayout.h.md)
- [`kernel/memory/layout/physical_memory_layout.cppm`](memory/layout/physical_memory_layout.cppm.md)
- [`kernel/memory/memory.cppm`](memory/memory.cppm.md)
- [`kernel/memory/paging/kernel_page_table.cpp`](memory/paging/kernel_page_table.cpp.md)
- [`kernel/memory/paging/kernel_page_table.cppm`](memory/paging/kernel_page_table.cppm.md)
- [`kernel/memory/paging/sv39_page_table.cpp`](memory/paging/sv39_page_table.cpp.md)
- [`kernel/memory/paging/sv39_page_table.cppm`](memory/paging/sv39_page_table.cppm.md)
- [`kernel/memory/paging/user_paging.cppm`](memory/paging/user_paging.cppm.md)
- [`kernel/memory/paging/vm.cpp`](memory/paging/vm.cpp.md)
- [`kernel/memory/paging/vm.h`](memory/paging/vm.h.md)


<a id="process"></a>

## process

- [`kernel/process/arch/cpu.cpp`](process/arch/cpu.cpp.md)
- [`kernel/process/arch/cpu.cppm`](process/arch/cpu.cppm.md)
- [`kernel/process/arch/cpu_affinity.cpp`](process/arch/cpu_affinity.cpp.md)
- [`kernel/process/arch/registers.cppm`](process/arch/registers.cppm.md)
- [`kernel/process/arch/swtch.S`](process/arch/swtch.S.md)
- [`kernel/process/exec/elf_format.cppm`](process/exec/elf_format.cppm.md)
- [`kernel/process/exec/elf_loader.cpp`](process/exec/elf_loader.cpp.md)
- [`kernel/process/exec/elf_loader.cppm`](process/exec/elf_loader.cppm.md)
- [`kernel/process/exec/executable_loader.cppm`](process/exec/executable_loader.cppm.md)
- [`kernel/process/groups/group_operations.cpp`](process/groups/group_operations.cpp.md)
- [`kernel/process/groups/kernel_platform.cpp`](process/groups/kernel_platform.cpp.md)
- [`kernel/process/groups/platform.cppm`](process/groups/platform.cppm.md)
- [`kernel/process/groups/process_groups.cpp`](process/groups/process_groups.cpp.md)
- [`kernel/process/groups/process_groups.cppm`](process/groups/process_groups.cppm.md)
- [`kernel/process/groups/process_membership.cppm`](process/groups/process_membership.cppm.md)
- [`kernel/process/groups/terminal_control.cpp`](process/groups/terminal_control.cpp.md)
- [`kernel/process/lifecycle/first_return.cpp`](process/lifecycle/first_return.cpp.md)
- [`kernel/process/lifecycle/process.cpp`](process/lifecycle/process.cpp.md)
- [`kernel/process/lifecycle/process_manager.cpp`](process/lifecycle/process_manager.cpp.md)
- [`kernel/process/lifecycle/process_manager.cppm`](process/lifecycle/process_manager.cppm.md)
- [`kernel/process/lifecycle/service.cpp`](process/lifecycle/service.cpp.md)
- [`kernel/process/memory/address_space.cpp`](process/memory/address_space.cpp.md)
- [`kernel/process/memory/address_space.cppm`](process/memory/address_space.cppm.md)
- [`kernel/process/process.cppm`](process/process.cppm.md)
- [`kernel/process/process_module.cppm`](process/process_module.cppm.md)
- [`kernel/process/resources/kernel_platform.cpp`](process/resources/kernel_platform.cpp.md)
- [`kernel/process/resources/process_resources.cpp`](process/resources/process_resources.cpp.md)
- [`kernel/process/resources/process_resources.cppm`](process/resources/process_resources.cppm.md)
- [`kernel/process/resources/resource_platform.cppm`](process/resources/resource_platform.cppm.md)
- [`kernel/process/scheduler/policies/round_robin.cpp`](process/scheduler/policies/round_robin.cpp.md)
- [`kernel/process/scheduler/policies/round_robin.cppm`](process/scheduler/policies/round_robin.cppm.md)
- [`kernel/process/scheduler/queues/fifo_ready_queue.cpp`](process/scheduler/queues/fifo_ready_queue.cpp.md)
- [`kernel/process/scheduler/queues/fifo_ready_queue.cppm`](process/scheduler/queues/fifo_ready_queue.cppm.md)
- [`kernel/process/scheduler/queues/ready_queue.cpp`](process/scheduler/queues/ready_queue.cpp.md)
- [`kernel/process/scheduler/queues/ready_queue.cppm`](process/scheduler/queues/ready_queue.cppm.md)
- [`kernel/process/scheduler/runtime/scheduler_context.cpp`](process/scheduler/runtime/scheduler_context.cpp.md)
- [`kernel/process/scheduler/runtime/scheduler_context.cppm`](process/scheduler/runtime/scheduler_context.cppm.md)
- [`kernel/process/scheduler/scheduler.cppm`](process/scheduler/scheduler.cppm.md)
- [`kernel/process/signals/pending_signals.cppm`](process/signals/pending_signals.cppm.md)
- [`kernel/process/signals/process_signals.cpp`](process/signals/process_signals.cpp.md)
- [`kernel/process/signals/signal.h`](process/signals/signal.h.md)
- [`kernel/process/signals/signal_abi.h`](process/signals/signal_abi.h.md)
- [`kernel/process/signals/signal_delivery.cpp`](process/signals/signal_delivery.cpp.md)
- [`kernel/process/signals/user_signals.cpp`](process/signals/user_signals.cpp.md)
- [`kernel/process/storage/kernel_platform.cpp`](process/storage/kernel_platform.cpp.md)
- [`kernel/process/storage/paged_process_store.cpp`](process/storage/paged_process_store.cpp.md)
- [`kernel/process/storage/paged_process_store.cppm`](process/storage/paged_process_store.cppm.md)
- [`kernel/process/storage/platform.cppm`](process/storage/platform.cppm.md)
- [`kernel/process/storage/process_store.cpp`](process/storage/process_store.cpp.md)
- [`kernel/process/storage/process_store.cppm`](process/storage/process_store.cppm.md)
- [`kernel/process/wait/poll_entry.cppm`](process/wait/poll_entry.cppm.md)
- [`kernel/process/wait/poll_wait.cpp`](process/wait/poll_wait.cpp.md)
- [`kernel/process/wait/poll_wait.cppm`](process/wait/poll_wait.cppm.md)
- [`kernel/process/wait/sleep.cpp`](process/wait/sleep.cpp.md)
- [`kernel/process/wait/wait.h`](process/wait/wait.h.md)
- [`kernel/process/wait/wait_channels.cpp`](process/wait/wait_channels.cpp.md)
- [`kernel/process/wait/wait_channels.cppm`](process/wait/wait_channels.cppm.md)
- [`kernel/process/wait/wait_child.cpp`](process/wait/wait_child.cpp.md)


<a id="support"></a>

## support

- [`kernel/support/compiler_memory.cpp`](support/compiler_memory.cpp.md)
- [`kernel/support/cpp_runtime.cpp`](support/cpp_runtime.cpp.md)
- [`kernel/support/param.h`](support/param.h.md)
- [`kernel/support/placement_new.hpp`](support/placement_new.hpp.md)
- [`kernel/support/print.cppm`](support/print.cppm.md)
- [`kernel/support/printk.cpp`](support/printk.cpp.md)
- [`kernel/support/runtime.cppm`](support/runtime.cppm.md)
- [`kernel/support/stats.cpp`](support/stats.cpp.md)
- [`kernel/support/string.cpp`](support/string.cpp.md)
- [`kernel/support/types.h`](support/types.h.md)


<a id="sync"></a>

## sync

- [`kernel/sync/interrupts.cppm`](sync/interrupts.cppm.md)
- [`kernel/sync/lock.cppm`](sync/lock.cppm.md)
- [`kernel/sync/platform/kernel_bridge.cpp`](sync/platform/kernel_bridge.cpp.md)
- [`kernel/sync/platform/kernel_platform.cpp`](sync/platform/kernel_platform.cpp.md)
- [`kernel/sync/platform/platform.cppm`](sync/platform/platform.cppm.md)
- [`kernel/sync/poll_table.cppm`](sync/poll_table.cppm.md)
- [`kernel/sync/scoped_lock.cppm`](sync/scoped_lock.cppm.md)
- [`kernel/sync/sleep/sleeplock.cpp`](sync/sleep/sleeplock.cpp.md)
- [`kernel/sync/sleep/sleeplock.cppm`](sync/sleep/sleeplock.cppm.md)
- [`kernel/sync/spin/spinlock.cpp`](sync/spin/spinlock.cpp.md)
- [`kernel/sync/spin/spinlock.cppm`](sync/spin/spinlock.cppm.md)
- [`kernel/sync/sync.cppm`](sync/sync.cppm.md)


<a id="syscall"></a>

## syscall

- [`kernel/syscall/arguments.cpp`](syscall/arguments.cpp.md)
- [`kernel/syscall/arguments.cppm`](syscall/arguments.cppm.md)
- [`kernel/syscall/handlers.cppm`](syscall/handlers.cppm.md)
- [`kernel/syscall/syscall.cpp`](syscall/syscall.cpp.md)
- [`kernel/syscall/syscall.cppm`](syscall/syscall.cppm.md)
- [`kernel/syscall/syscall.h`](syscall/syscall.h.md)
- [`kernel/syscall/sysfile.cpp`](syscall/sysfile.cpp.md)
- [`kernel/syscall/syspoll.cpp`](syscall/syspoll.cpp.md)
- [`kernel/syscall/sysproc.cpp`](syscall/sysproc.cpp.md)
- [`kernel/syscall/sysvm.cpp`](syscall/sysvm.cpp.md)

- [`kernel/syscall/sysnet.cpp`](syscall/sysnet.cpp.md)

<a id="trap"></a>

## trap

- [`kernel/trap/arch/riscv/entry.cpp`](trap/arch/riscv/entry.cpp.md)
- [`kernel/trap/arch/riscv/entry.h`](trap/arch/riscv/entry.h.md)
- [`kernel/trap/arch/riscv/kernel_platform.cpp`](trap/arch/riscv/kernel_platform.cpp.md)
- [`kernel/trap/arch/riscv/kernelvec.S`](trap/arch/riscv/kernelvec.S.md)
- [`kernel/trap/arch/riscv/trampoline.S`](trap/arch/riscv/trampoline.S.md)
- [`kernel/trap/clock/tick_clock.cpp`](trap/clock/tick_clock.cpp.md)
- [`kernel/trap/clock/tick_clock.cppm`](trap/clock/tick_clock.cppm.md)
- [`kernel/trap/controller/trap_controller.cpp`](trap/controller/trap_controller.cpp.md)
- [`kernel/trap/controller/trap_controller.cppm`](trap/controller/trap_controller.cppm.md)
- [`kernel/trap/dispatch/trap_dispatcher.cpp`](trap/dispatch/trap_dispatcher.cpp.md)
- [`kernel/trap/dispatch/trap_dispatcher.cppm`](trap/dispatch/trap_dispatcher.cppm.md)
- [`kernel/trap/event/trap_event.cppm`](trap/event/trap_event.cppm.md)
- [`kernel/trap/handlers/external_interrupt.cpp`](trap/handlers/external_interrupt.cpp.md)
- [`kernel/trap/handlers/handlers.cppm`](trap/handlers/handlers.cppm.md)
- [`kernel/trap/handlers/page_fault.cpp`](trap/handlers/page_fault.cpp.md)
- [`kernel/trap/handlers/system_call.cpp`](trap/handlers/system_call.cpp.md)
- [`kernel/trap/handlers/timer_interrupt.cpp`](trap/handlers/timer_interrupt.cpp.md)
- [`kernel/trap/platform/platform.cppm`](trap/platform/platform.cppm.md)
- [`kernel/trap/service.cpp`](trap/service.cpp.md)
- [`kernel/trap/trap.cppm`](trap/trap.cppm.md)


<a id="tty"></a>

## tty

- [`kernel/tty/termios.h`](tty/termios.h.md)



<a id="net"></a>

## net

- [`kernel/net/net.cppm`](net/net.cppm.md)
- [`kernel/net/platform/kernel_platform.cpp`](net/platform/kernel_platform.cpp.md)
- [`kernel/net/platform/platform.cppm`](net/platform/platform.cppm.md)
- [`kernel/net/stack.cpp`](net/stack.cpp.md)
