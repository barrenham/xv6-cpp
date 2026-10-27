# kernel 全量函数开发文档

函数说明与运行代码分离，按 USR 维护单一函数条目。

| 子系统 | 文件数 | 函数/入口条目数 |
| --- | ---: | ---: |
| [arch](#arch) | 4 | 49 |
| [boot](#boot) | 6 | 10 |
| [drivers](#drivers) | 33 | 238 |
| [fs](#fs) | 39 | 401 |
| [io](#io) | 2 | 19 |
| [memory](#memory) | 24 | 199 |
| [net](#net) | 60 | 360 |
| [process](#process) | 59 | 346 |
| [support](#support) | 10 | 32 |
| [sync](#sync) | 12 | 47 |
| [syscall](#syscall) | 12 | 102 |
| [trap](#trap) | 20 | 93 |
| [tty](#tty) | 1 | 0 |

范围：282 个文件；默认 1831 个唯一 C++ 函数，连同条件接口与汇编入口共 1896 条文档。实际 hover 能力以当前 clangd 验证报告为准。

<a id="arch"></a>

## arch

- [`kernel/arch/arch.cppm`](arch/arch.cppm.md)
- [`kernel/arch/riscv/register_operations.inc`](arch/riscv/register_operations.inc.md)
- [`kernel/arch/riscv/register_constants.h`](arch/riscv/register_constants.h.md)
- [`kernel/arch/riscv/riscv.h`](arch/riscv/riscv.h.md)

<a id="boot"></a>

## boot

- [`kernel/boot/main.cpp`](boot/main.cpp.md)
- [`kernel/boot/entry.cpp`](boot/entry.cpp.md)
- [`kernel/boot/start.cpp`](boot/start.cpp.md)
- [`kernel/boot/boot.cppm`](boot/boot.cppm.md)
- [`kernel/boot/state.cppm`](boot/state.cppm.md)
- [`kernel/boot/entry.S`](boot/entry.S.md)

<a id="drivers"></a>

## drivers

- [`kernel/drivers/device.cppm`](drivers/device.cppm.md)
- [`kernel/drivers/mmio.cppm`](drivers/mmio.cppm.md)
- [`kernel/drivers/device_numbers.h`](drivers/device_numbers.h.md)
- [`kernel/drivers/drivers.cppm`](drivers/drivers.cppm.md)
- [`kernel/drivers/platform/platform.cppm`](drivers/platform/platform.cppm.md)
- [`kernel/drivers/platform/kernel_platform.cpp`](drivers/platform/kernel_platform.cpp.md)
- [`kernel/drivers/net/ethernet.cppm`](drivers/net/ethernet.cppm.md)
- [`kernel/drivers/console/character_device.cppm`](drivers/console/character_device.cppm.md)
- [`kernel/drivers/interrupt/interrupt_controller.cppm`](drivers/interrupt/interrupt_controller.cppm.md)
- [`kernel/drivers/interrupt/interrupt_router.cpp`](drivers/interrupt/interrupt_router.cpp.md)
- [`kernel/drivers/interrupt/plic.cppm`](drivers/interrupt/plic.cppm.md)
- [`kernel/drivers/interrupt/plic.cpp`](drivers/interrupt/plic.cpp.md)
- [`kernel/drivers/interrupt/interrupt_router.cppm`](drivers/interrupt/interrupt_router.cppm.md)
- [`kernel/drivers/serial/uart16550.cppm`](drivers/serial/uart16550.cppm.md)
- [`kernel/drivers/serial/serial_port.cppm`](drivers/serial/serial_port.cppm.md)
- [`kernel/drivers/serial/uart16550.cpp`](drivers/serial/uart16550.cpp.md)
- [`kernel/drivers/pci/qemu_pci.cppm`](drivers/pci/qemu_pci.cppm.md)
- [`kernel/drivers/pci/qemu_pci.cpp`](drivers/pci/qemu_pci.cpp.md)
- [`kernel/drivers/block/block_device.cppm`](drivers/block/block_device.cppm.md)
- [`kernel/drivers/board/qemu_virt.cpp`](drivers/board/qemu_virt.cpp.md)
- [`kernel/drivers/block/virtio/protocol.cppm`](drivers/block/virtio/protocol.cppm.md)
- [`kernel/drivers/block/virtio/virtio_block.cppm`](drivers/block/virtio/virtio_block.cppm.md)
- [`kernel/drivers/block/virtio/virtio_block.cpp`](drivers/block/virtio/virtio_block.cpp.md)
- [`kernel/drivers/net/e1000/e1000.cpp`](drivers/net/e1000/e1000.cpp.md)
- [`kernel/drivers/net/e1000/e1000.cppm`](drivers/net/e1000/e1000.cppm.md)
- [`kernel/drivers/tty/pty/pty.cpp`](drivers/tty/pty/pty.cpp.md)
- [`kernel/drivers/tty/pty/pty.cppm`](drivers/tty/pty/pty.cppm.md)
- [`kernel/drivers/tty/ldisc/n_tty.cppm`](drivers/tty/ldisc/n_tty.cppm.md)
- [`kernel/drivers/tty/ldisc/line_discipline.cppm`](drivers/tty/ldisc/line_discipline.cppm.md)
- [`kernel/drivers/tty/ldisc/n_tty.cpp`](drivers/tty/ldisc/n_tty.cpp.md)
- [`kernel/drivers/tty/core/tty.cppm`](drivers/tty/core/tty.cppm.md)
- [`kernel/drivers/tty/core/tty.cpp`](drivers/tty/core/tty.cpp.md)
- [`kernel/drivers/tty/core/terminal_output.cppm`](drivers/tty/core/terminal_output.cppm.md)

<a id="fs"></a>

## fs

- [`kernel/fs/service.cpp`](fs/service.cpp.md)
- [`kernel/fs/fs.cppm`](fs/fs.cppm.md)
- [`kernel/fs/image_paths.h`](fs/image_paths.h.md)
- [`kernel/fs/fcntl.h`](fs/fcntl.h.md)
- [`kernel/fs/stat.h`](fs/stat.h.md)
- [`kernel/fs/poll.h`](fs/poll.h.md)
- [`kernel/fs/io_status.h`](fs/io_status.h.md)
- [`kernel/fs/format/layout.h`](fs/format/layout.h.md)
- [`kernel/fs/file/file_operations.cppm`](fs/file/file_operations.cppm.md)
- [`kernel/fs/file/file_operations.cpp`](fs/file/file_operations.cpp.md)
- [`kernel/fs/file/open_file.cpp`](fs/file/open_file.cpp.md)
- [`kernel/fs/file/open_file.cppm`](fs/file/open_file.cppm.md)
- [`kernel/fs/journal/journal.cpp`](fs/journal/journal.cpp.md)
- [`kernel/fs/journal/journal.cppm`](fs/journal/journal.cppm.md)
- [`kernel/fs/platform/platform.cppm`](fs/platform/platform.cppm.md)
- [`kernel/fs/platform/kernel_platform.cpp`](fs/platform/kernel_platform.cpp.md)
- [`kernel/fs/pipe/pipe.cpp`](fs/pipe/pipe.cpp.md)
- [`kernel/fs/pipe/pipe.cppm`](fs/pipe/pipe.cppm.md)
- [`kernel/fs/pipe/pipe_factory.cpp`](fs/pipe/pipe_factory.cpp.md)
- [`kernel/fs/path/path_operations.cpp`](fs/path/path_operations.cpp.md)
- [`kernel/fs/cache/lru_block_cache.cpp`](fs/cache/lru_block_cache.cpp.md)
- [`kernel/fs/cache/block_cache.cppm`](fs/cache/block_cache.cppm.md)
- [`kernel/fs/cache/lru_block_cache.cppm`](fs/cache/lru_block_cache.cppm.md)
- [`kernel/fs/inode/xv6_filesystem.cppm`](fs/inode/xv6_filesystem.cppm.md)
- [`kernel/fs/inode/filesystem.cppm`](fs/inode/filesystem.cppm.md)
- [`kernel/fs/inode/inode.cppm`](fs/inode/inode.cppm.md)
- [`kernel/fs/inode/xv6_filesystem.cpp`](fs/inode/xv6_filesystem.cpp.md)
- [`kernel/fs/devfs/devfs.cppm`](fs/devfs/devfs.cppm.md)
- [`kernel/fs/devfs/devfs.cpp`](fs/devfs/devfs.cpp.md)
- [`kernel/fs/vfs/vfs.cpp`](fs/vfs/vfs.cpp.md)
- [`kernel/fs/vfs/vfs.cppm`](fs/vfs/vfs.cppm.md)
- [`kernel/fs/procfs/procfs.cpp`](fs/procfs/procfs.cpp.md)
- [`kernel/fs/procfs/procfs.cppm`](fs/procfs/procfs.cppm.md)
- [`kernel/fs/buffer/page_buffer.cpp`](fs/buffer/page_buffer.cpp.md)
- [`kernel/fs/buffer/page_buffer.cppm`](fs/buffer/page_buffer.cppm.md)
- [`kernel/fs/ramfs/ramfs.cpp`](fs/ramfs/ramfs.cpp.md)
- [`kernel/fs/ramfs/ramfs.cppm`](fs/ramfs/ramfs.cppm.md)
- [`kernel/fs/permissions.cpp`](fs/permissions.cpp.md)
- [`kernel/fs/mount.h`](fs/mount.h.md)

<a id="io"></a>

## io

- [`kernel/io/object.cppm`](io/object.cppm.md)
- [`kernel/io/stat.h`](io/stat.h.md)

<a id="memory"></a>

## memory

- [`kernel/memory/memory.cppm`](memory/memory.cppm.md)
- [`kernel/memory/layout/memlayout.h`](memory/layout/memlayout.h.md)
- [`kernel/memory/layout/physical_memory_layout.cppm`](memory/layout/physical_memory_layout.cppm.md)
- [`kernel/memory/layout/virtual_regions.cppm`](memory/layout/virtual_regions.cppm.md)
- [`kernel/memory/layout/virtual_regions.cpp`](memory/layout/virtual_regions.cpp.md)
- [`kernel/memory/address/physical_memory_range.cppm`](memory/address/physical_memory_range.cppm.md)
- [`kernel/memory/address/physical_address.cppm`](memory/address/physical_address.cppm.md)
- [`kernel/memory/allocator/allocator_service.cpp`](memory/allocator/allocator_service.cpp.md)
- [`kernel/memory/allocator/page_allocator.cppm`](memory/allocator/page_allocator.cppm.md)
- [`kernel/memory/allocator/allocator_detail.cppm`](memory/allocator/allocator_detail.cppm.md)
- [`kernel/memory/boot/boot_memory.cpp`](memory/boot/boot_memory.cpp.md)
- [`kernel/memory/boot/kalloc.cpp`](memory/boot/kalloc.cpp.md)
- [`kernel/memory/boot/boot_memory.cppm`](memory/boot/boot_memory.cppm.md)
- [`kernel/memory/paging/vm.h`](memory/paging/vm.h.md)
- [`kernel/memory/paging/vm.cpp`](memory/paging/vm.cpp.md)
- [`kernel/memory/paging/kernel_page_table.cppm`](memory/paging/kernel_page_table.cppm.md)
- [`kernel/memory/paging/sv39_page_table.cppm`](memory/paging/sv39_page_table.cppm.md)
- [`kernel/memory/paging/sv39_page_table.cpp`](memory/paging/sv39_page_table.cpp.md)
- [`kernel/memory/paging/kernel_page_table.cpp`](memory/paging/kernel_page_table.cpp.md)
- [`kernel/memory/paging/user_paging.cppm`](memory/paging/user_paging.cppm.md)
- [`kernel/memory/allocator/buddy/buddy_page_allocator.cpp`](memory/allocator/buddy/buddy_page_allocator.cpp.md)
- [`kernel/memory/allocator/buddy/buddy_page_allocator.cppm`](memory/allocator/buddy/buddy_page_allocator.cppm.md)
- [`kernel/memory/allocator/freelist/freelist_page_allocator.cppm`](memory/allocator/freelist/freelist_page_allocator.cppm.md)
- [`kernel/memory/allocator/freelist/freelist_page_allocator.cpp`](memory/allocator/freelist/freelist_page_allocator.cpp.md)

<a id="net"></a>

## net

- [`kernel/net/net.cppm`](net/net.cppm.md)
- [`kernel/net/errno.h`](net/errno.h.md)
- [`kernel/net/control.h`](net/control.h.md)
- [`kernel/net/link/arp.cpp`](net/link/arp.cpp.md)
- [`kernel/net/link/arp.cppm`](net/link/arp.cppm.md)
- [`kernel/net/link/ethernet.cpp`](net/link/ethernet.cpp.md)
- [`kernel/net/link/port.cppm`](net/link/port.cppm.md)
- [`kernel/net/link/ethernet.cppm`](net/link/ethernet.cppm.md)
- [`kernel/net/interface/loopback.cpp`](net/interface/loopback.cpp.md)
- [`kernel/net/interface/ethernet_interface.cpp`](net/interface/ethernet_interface.cpp.md)
- [`kernel/net/interface/configuration.cpp`](net/interface/configuration.cpp.md)
- [`kernel/net/interface/interface.cppm`](net/interface/interface.cppm.md)
- [`kernel/net/interface/configuration.cppm`](net/interface/configuration.cppm.md)
- [`kernel/net/interface/ethernet_interface.cppm`](net/interface/ethernet_interface.cppm.md)
- [`kernel/net/interface/interface.cpp`](net/interface/interface.cpp.md)
- [`kernel/net/interface/loopback.cppm`](net/interface/loopback.cppm.md)
- [`kernel/net/interface/tunnel.cppm`](net/interface/tunnel.cppm.md)
- [`kernel/net/interface/tunnel.cpp`](net/interface/tunnel.cpp.md)
- [`kernel/net/platform/platform.cppm`](net/platform/platform.cppm.md)
- [`kernel/net/platform/kernel_platform.cpp`](net/platform/kernel_platform.cpp.md)
- [`kernel/net/ip/routing.cpp`](net/ip/routing.cpp.md)
- [`kernel/net/ip/ipv4.cpp`](net/ip/ipv4.cpp.md)
- [`kernel/net/ip/icmp.cppm`](net/ip/icmp.cppm.md)
- [`kernel/net/ip/icmp.cpp`](net/ip/icmp.cpp.md)
- [`kernel/net/ip/routing.cppm`](net/ip/routing.cppm.md)
- [`kernel/net/ip/ipv4.cppm`](net/ip/ipv4.cppm.md)
- [`kernel/net/transport/retransmission_timer.cppm`](net/transport/retransmission_timer.cppm.md)
- [`kernel/net/transport/tcp.cppm`](net/transport/tcp.cppm.md)
- [`kernel/net/transport/udp.cpp`](net/transport/udp.cpp.md)
- [`kernel/net/transport/tcp_protocol.cpp`](net/transport/tcp_protocol.cpp.md)
- [`kernel/net/transport/udp.cppm`](net/transport/udp.cppm.md)
- [`kernel/net/socket/udp_socket.cppm`](net/socket/udp_socket.cppm.md)
- [`kernel/net/socket/inet_socket.cpp`](net/socket/inet_socket.cpp.md)
- [`kernel/net/socket/registry.cppm`](net/socket/registry.cppm.md)
- [`kernel/net/socket/tcp_socket.cppm`](net/socket/tcp_socket.cppm.md)
- [`kernel/net/socket/registry.cpp`](net/socket/registry.cpp.md)
- [`kernel/net/socket/udp_socket.cpp`](net/socket/udp_socket.cpp.md)
- [`kernel/net/socket/tcp_socket.cpp`](net/socket/tcp_socket.cpp.md)
- [`kernel/net/socket/socket.cppm`](net/socket/socket.cppm.md)
- [`kernel/net/socket/socket.h`](net/socket/socket.h.md)
- [`kernel/net/bridge/bridge.cpp`](net/bridge/bridge.cpp.md)
- [`kernel/net/bridge/manager.cpp`](net/bridge/manager.cpp.md)
- [`kernel/net/bridge/bridge.cppm`](net/bridge/bridge.cppm.md)
- [`kernel/net/core/wire.cppm`](net/core/wire.cppm.md)
- [`kernel/net/core/service.cpp`](net/core/service.cpp.md)
- [`kernel/net/core/control.cppm`](net/core/control.cppm.md)
- [`kernel/net/core/packet.cppm`](net/core/packet.cppm.md)
- [`kernel/net/core/packet.cpp`](net/core/packet.cpp.md)
- [`kernel/net/core/control.cpp`](net/core/control.cpp.md)
- [`kernel/net/core/address.cppm`](net/core/address.cppm.md)
- [`kernel/net/dhcp/client.cpp`](net/dhcp/client.cpp.md)
- [`kernel/net/dhcp/client.cppm`](net/dhcp/client.cppm.md)
- [`kernel/net/dhcp/lease.cpp`](net/dhcp/lease.cpp.md)
- [`kernel/net/dhcp/lease.cppm`](net/dhcp/lease.cppm.md)
- [`kernel/net/dhcp/message.cpp`](net/dhcp/message.cpp.md)
- [`kernel/net/dhcp/message.cppm`](net/dhcp/message.cppm.md)
- [`kernel/net/transport/stream_buffer.cppm`](net/transport/stream_buffer.cppm.md)
- [`kernel/net/transport/congestion.cppm`](net/transport/congestion.cppm.md)
- [`kernel/net/socket/tcp_sender.cpp`](net/socket/tcp_sender.cpp.md)
- [`kernel/net/socket/tcp_receiver.cpp`](net/socket/tcp_receiver.cpp.md)

<a id="process"></a>

## process

- [`kernel/process/process.cppm`](process/process.cppm.md)
- [`kernel/process/process_module.cppm`](process/process_module.cppm.md)
- [`kernel/process/scheduler/scheduler.cppm`](process/scheduler/scheduler.cppm.md)
- [`kernel/process/signals/user_signals.cpp`](process/signals/user_signals.cpp.md)
- [`kernel/process/signals/process_signals.cpp`](process/signals/process_signals.cpp.md)
- [`kernel/process/signals/pending_signals.cppm`](process/signals/pending_signals.cppm.md)
- [`kernel/process/signals/signal_abi.h`](process/signals/signal_abi.h.md)
- [`kernel/process/signals/signal.h`](process/signals/signal.h.md)
- [`kernel/process/signals/signal_delivery.cpp`](process/signals/signal_delivery.cpp.md)
- [`kernel/process/wait/poll_entry.cppm`](process/wait/poll_entry.cppm.md)
- [`kernel/process/wait/poll_wait.cpp`](process/wait/poll_wait.cpp.md)
- [`kernel/process/wait/poll_wait.cppm`](process/wait/poll_wait.cppm.md)
- [`kernel/process/wait/wait_child.cpp`](process/wait/wait_child.cpp.md)
- [`kernel/process/wait/wait_channels.cpp`](process/wait/wait_channels.cpp.md)
- [`kernel/process/wait/wait.h`](process/wait/wait.h.md)
- [`kernel/process/wait/wait_channels.cppm`](process/wait/wait_channels.cppm.md)
- [`kernel/process/wait/sleep.cpp`](process/wait/sleep.cpp.md)
- [`kernel/process/storage/platform.cppm`](process/storage/platform.cppm.md)
- [`kernel/process/storage/paged_process_store.cppm`](process/storage/paged_process_store.cppm.md)
- [`kernel/process/storage/process_store.cpp`](process/storage/process_store.cpp.md)
- [`kernel/process/storage/paged_process_store.cpp`](process/storage/paged_process_store.cpp.md)
- [`kernel/process/storage/kernel_platform.cpp`](process/storage/kernel_platform.cpp.md)
- [`kernel/process/storage/process_store.cppm`](process/storage/process_store.cppm.md)
- [`kernel/process/exec/elf_loader.cppm`](process/exec/elf_loader.cppm.md)
- [`kernel/process/exec/executable_loader.cppm`](process/exec/executable_loader.cppm.md)
- [`kernel/process/exec/elf_loader.cpp`](process/exec/elf_loader.cpp.md)
- [`kernel/process/exec/elf_format.cppm`](process/exec/elf_format.cppm.md)
- [`kernel/process/arch/cpu.cppm`](process/arch/cpu.cppm.md)
- [`kernel/process/arch/cpu_affinity.cpp`](process/arch/cpu_affinity.cpp.md)
- [`kernel/process/arch/cpu.cpp`](process/arch/cpu.cpp.md)
- [`kernel/process/arch/swtch.S`](process/arch/swtch.S.md)
- [`kernel/process/arch/registers.cppm`](process/arch/registers.cppm.md)
- [`kernel/process/groups/process_groups.cppm`](process/groups/process_groups.cppm.md)
- [`kernel/process/groups/platform.cppm`](process/groups/platform.cppm.md)
- [`kernel/process/groups/process_membership.cppm`](process/groups/process_membership.cppm.md)
- [`kernel/process/groups/group_operations.cpp`](process/groups/group_operations.cpp.md)
- [`kernel/process/groups/terminal_control.cpp`](process/groups/terminal_control.cpp.md)
- [`kernel/process/groups/kernel_platform.cpp`](process/groups/kernel_platform.cpp.md)
- [`kernel/process/groups/process_groups.cpp`](process/groups/process_groups.cpp.md)
- [`kernel/process/memory/address_space.cppm`](process/memory/address_space.cppm.md)
- [`kernel/process/memory/address_space.cpp`](process/memory/address_space.cpp.md)
- [`kernel/process/lifecycle/service.cpp`](process/lifecycle/service.cpp.md)
- [`kernel/process/lifecycle/first_return.cpp`](process/lifecycle/first_return.cpp.md)
- [`kernel/process/lifecycle/process.cpp`](process/lifecycle/process.cpp.md)
- [`kernel/process/lifecycle/process_manager.cppm`](process/lifecycle/process_manager.cppm.md)
- [`kernel/process/lifecycle/process_manager.cpp`](process/lifecycle/process_manager.cpp.md)
- [`kernel/process/resources/process_resources.cppm`](process/resources/process_resources.cppm.md)
- [`kernel/process/resources/kernel_platform.cpp`](process/resources/kernel_platform.cpp.md)
- [`kernel/process/resources/process_resources.cpp`](process/resources/process_resources.cpp.md)
- [`kernel/process/resources/resource_platform.cppm`](process/resources/resource_platform.cppm.md)
- [`kernel/process/scheduler/policies/round_robin.cppm`](process/scheduler/policies/round_robin.cppm.md)
- [`kernel/process/scheduler/policies/round_robin.cpp`](process/scheduler/policies/round_robin.cpp.md)
- [`kernel/process/scheduler/runtime/scheduler_context.cppm`](process/scheduler/runtime/scheduler_context.cppm.md)
- [`kernel/process/scheduler/runtime/scheduler_context.cpp`](process/scheduler/runtime/scheduler_context.cpp.md)
- [`kernel/process/scheduler/queues/ready_queue.cpp`](process/scheduler/queues/ready_queue.cpp.md)
- [`kernel/process/scheduler/queues/fifo_ready_queue.cpp`](process/scheduler/queues/fifo_ready_queue.cpp.md)
- [`kernel/process/scheduler/queues/fifo_ready_queue.cppm`](process/scheduler/queues/fifo_ready_queue.cppm.md)
- [`kernel/process/scheduler/queues/ready_queue.cppm`](process/scheduler/queues/ready_queue.cppm.md)
- [`kernel/process/credentials/credentials.cppm`](process/credentials/credentials.cppm.md)

<a id="support"></a>

## support

- [`kernel/support/placement_new.hpp`](support/placement_new.hpp.md)
- [`kernel/support/types.h`](support/types.h.md)
- [`kernel/support/stats.cpp`](support/stats.cpp.md)
- [`kernel/support/string.cpp`](support/string.cpp.md)
- [`kernel/support/printk.cpp`](support/printk.cpp.md)
- [`kernel/support/compiler_memory.cpp`](support/compiler_memory.cpp.md)
- [`kernel/support/print.cppm`](support/print.cppm.md)
- [`kernel/support/runtime.cppm`](support/runtime.cppm.md)
- [`kernel/support/cpp_runtime.cpp`](support/cpp_runtime.cpp.md)
- [`kernel/support/param.h`](support/param.h.md)

<a id="sync"></a>

## sync

- [`kernel/sync/lock.cppm`](sync/lock.cppm.md)
- [`kernel/sync/scoped_lock.cppm`](sync/scoped_lock.cppm.md)
- [`kernel/sync/interrupts.cppm`](sync/interrupts.cppm.md)
- [`kernel/sync/poll_table.cppm`](sync/poll_table.cppm.md)
- [`kernel/sync/sync.cppm`](sync/sync.cppm.md)
- [`kernel/sync/spin/spinlock.cppm`](sync/spin/spinlock.cppm.md)
- [`kernel/sync/spin/spinlock.cpp`](sync/spin/spinlock.cpp.md)
- [`kernel/sync/platform/kernel_bridge.cpp`](sync/platform/kernel_bridge.cpp.md)
- [`kernel/sync/platform/platform.cppm`](sync/platform/platform.cppm.md)
- [`kernel/sync/platform/kernel_platform.cpp`](sync/platform/kernel_platform.cpp.md)
- [`kernel/sync/sleep/sleeplock.cpp`](sync/sleep/sleeplock.cpp.md)
- [`kernel/sync/sleep/sleeplock.cppm`](sync/sleep/sleeplock.cppm.md)

<a id="syscall"></a>

## syscall

- [`kernel/syscall/syscall.cppm`](syscall/syscall.cppm.md)
- [`kernel/syscall/syscall.cpp`](syscall/syscall.cpp.md)
- [`kernel/syscall/arguments.cppm`](syscall/arguments.cppm.md)
- [`kernel/syscall/handlers.cppm`](syscall/handlers.cppm.md)
- [`kernel/syscall/syscall.h`](syscall/syscall.h.md)
- [`kernel/syscall/sysnetctl.cpp`](syscall/sysnetctl.cpp.md)
- [`kernel/syscall/sysfile.cpp`](syscall/sysfile.cpp.md)
- [`kernel/syscall/arguments.cpp`](syscall/arguments.cpp.md)
- [`kernel/syscall/sysproc.cpp`](syscall/sysproc.cpp.md)
- [`kernel/syscall/syssocket.cpp`](syscall/syssocket.cpp.md)
- [`kernel/syscall/sysvm.cpp`](syscall/sysvm.cpp.md)
- [`kernel/syscall/syspoll.cpp`](syscall/syspoll.cpp.md)

<a id="trap"></a>

## trap

- [`kernel/trap/service.cpp`](trap/service.cpp.md)
- [`kernel/trap/trap.cppm`](trap/trap.cppm.md)
- [`kernel/trap/handlers/page_fault.cpp`](trap/handlers/page_fault.cpp.md)
- [`kernel/trap/handlers/timer_interrupt.cpp`](trap/handlers/timer_interrupt.cpp.md)
- [`kernel/trap/handlers/handlers.cppm`](trap/handlers/handlers.cppm.md)
- [`kernel/trap/handlers/system_call.cpp`](trap/handlers/system_call.cpp.md)
- [`kernel/trap/handlers/external_interrupt.cpp`](trap/handlers/external_interrupt.cpp.md)
- [`kernel/trap/platform/platform.cppm`](trap/platform/platform.cppm.md)
- [`kernel/trap/event/trap_event.cppm`](trap/event/trap_event.cppm.md)
- [`kernel/trap/dispatch/trap_dispatcher.cppm`](trap/dispatch/trap_dispatcher.cppm.md)
- [`kernel/trap/dispatch/trap_dispatcher.cpp`](trap/dispatch/trap_dispatcher.cpp.md)
- [`kernel/trap/controller/trap_controller.cppm`](trap/controller/trap_controller.cppm.md)
- [`kernel/trap/controller/trap_controller.cpp`](trap/controller/trap_controller.cpp.md)
- [`kernel/trap/clock/tick_clock.cppm`](trap/clock/tick_clock.cppm.md)
- [`kernel/trap/clock/tick_clock.cpp`](trap/clock/tick_clock.cpp.md)
- [`kernel/trap/arch/riscv/kernelvec.S`](trap/arch/riscv/kernelvec.S.md)
- [`kernel/trap/arch/riscv/entry.cpp`](trap/arch/riscv/entry.cpp.md)
- [`kernel/trap/arch/riscv/trampoline.S`](trap/arch/riscv/trampoline.S.md)
- [`kernel/trap/arch/riscv/entry.h`](trap/arch/riscv/entry.h.md)
- [`kernel/trap/arch/riscv/kernel_platform.cpp`](trap/arch/riscv/kernel_platform.cpp.md)

<a id="tty"></a>

## tty

- [`kernel/tty/termios.h`](tty/termios.h.md)
