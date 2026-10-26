cmake_minimum_required(VERSION 3.28)
project(xv_mkfs_host LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
# Keep output paths valid when CMake resets a native compiler cache during upgrades.
get_filename_component(default_output_dir "${CMAKE_BINARY_DIR}/.." ABSOLUTE)
set(XV_OUTPUT_DIR "${default_output_dir}" CACHE PATH "Native tool output directory")
add_executable(mkfs ../main.cpp)
target_compile_features(mkfs PRIVATE cxx_std_20)
target_compile_options(mkfs PRIVATE -O2 -Wall -Wextra -Werror)
target_include_directories(mkfs PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../..")
set_target_properties(mkfs PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${XV_OUTPUT_DIR}")

# Native contract tests use the same allocator/service sources as the kernel.
find_package(Threads REQUIRED)
set(XV_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../..")
add_executable(allocator_contract
  "${XV_ROOT}/tests/allocator_contract.cpp"
  "${XV_ROOT}/tests/allocator_test_platform.cpp"
  "${XV_ROOT}/kernel/memory/paging/sv39_page_table.cpp"
  "${XV_ROOT}/kernel/memory/boot/boot_memory.cpp"
  "${XV_ROOT}/kernel/memory/allocator/allocator_service.cpp"
  "${XV_ROOT}/kernel/memory/allocator/buddy/buddy_page_allocator.cpp"
  "${XV_ROOT}/kernel/memory/allocator/freelist/freelist_page_allocator.cpp")
target_include_directories(allocator_contract PRIVATE "${XV_ROOT}")
target_compile_features(allocator_contract PRIVATE cxx_std_20)
target_compile_options(allocator_contract PRIVATE -O2 -Wall -Wextra -Werror -fno-rtti -fno-exceptions)
target_link_libraries(allocator_contract PRIVATE Threads::Threads)
set_target_properties(allocator_contract PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${XV_OUTPUT_DIR}")

include("${XV_ROOT}/cmake/memory-modules.cmake")
xv_add_memory_modules(allocator_contract)

include("${XV_ROOT}/cmake/sync-modules.cmake")
xv_add_sync_modules(allocator_contract)
target_sources(allocator_contract PRIVATE
  "${XV_ROOT}/kernel/sync/spin/spinlock.cpp"
  "${XV_ROOT}/kernel/sync/sleep/sleeplock.cpp"
  "${XV_ROOT}/tests/sync_test_platform.cpp")

add_executable(sync_contract
  "${XV_ROOT}/tests/sync_contract.cpp"
  "${XV_ROOT}/tests/sync_test_platform.cpp"
  "${XV_ROOT}/kernel/sync/spin/spinlock.cpp"
  "${XV_ROOT}/kernel/sync/sleep/sleeplock.cpp")
target_compile_features(sync_contract PRIVATE cxx_std_20)
target_compile_options(sync_contract PRIVATE -O2 -Wall -Wextra -Werror -fno-rtti -fno-exceptions)
target_link_libraries(sync_contract PRIVATE Threads::Threads)
set_target_properties(sync_contract PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${XV_OUTPUT_DIR}")
xv_add_sync_modules(sync_contract)

include("${XV_ROOT}/cmake/driver-modules.cmake")
add_executable(driver_contract
  "${XV_ROOT}/tests/driver_contract.cpp"
  "${XV_ROOT}/tests/driver_test_platform.cpp"
  "${XV_ROOT}/tests/sync_test_platform.cpp"
  "${XV_ROOT}/kernel/sync/spin/spinlock.cpp"
  "${XV_ROOT}/kernel/sync/sleep/sleeplock.cpp"
  "${XV_ROOT}/kernel/drivers/serial/uart16550.cpp"
  "${XV_ROOT}/kernel/drivers/tty/core/tty.cpp"
  "${XV_ROOT}/kernel/drivers/tty/ldisc/n_tty.cpp"
  "${XV_ROOT}/kernel/drivers/block/virtio/virtio_block.cpp"
  "${XV_ROOT}/kernel/drivers/interrupt/plic.cpp"
  "${XV_ROOT}/kernel/drivers/interrupt/interrupt_router.cpp")
target_include_directories(driver_contract PRIVATE "${XV_ROOT}")
target_compile_features(driver_contract PRIVATE cxx_std_20)
target_compile_options(driver_contract PRIVATE -O2 -Wall -Wextra -Werror -fno-rtti -fno-exceptions)
target_link_libraries(driver_contract PRIVATE Threads::Threads)
set_target_properties(driver_contract PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${XV_OUTPUT_DIR}")
xv_add_sync_modules(driver_contract)
xv_add_driver_modules(driver_contract)

include("${XV_ROOT}/cmake/process-modules.cmake")
add_executable(process_contract
  "${XV_ROOT}/tests/process_contract.cpp"
  "${XV_ROOT}/tests/process_test_platform.cpp"
  "${XV_ROOT}/kernel/process/resources/process_resources.cpp")
target_include_directories(process_contract PRIVATE "${XV_ROOT}")
target_compile_features(process_contract PRIVATE cxx_std_20)
target_compile_options(process_contract PRIVATE -O2 -Wall -Wextra -Werror -fno-rtti -fno-exceptions)
set_target_properties(process_contract PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${XV_OUTPUT_DIR}")
xv_add_sync_modules(process_contract)
xv_add_process_interfaces(process_contract)
target_sources(process_contract PRIVATE
  "${XV_ROOT}/tests/sync_test_platform.cpp"
  "${XV_ROOT}/kernel/sync/spin/spinlock.cpp"
  "${XV_ROOT}/kernel/sync/sleep/sleeplock.cpp")
target_link_libraries(process_contract PRIVATE Threads::Threads)

include("${XV_ROOT}/cmake/fs-modules.cmake")
xv_add_driver_interfaces(process_contract)
xv_add_fs_interfaces(process_contract)

add_executable(fs_contract
  "${XV_ROOT}/tests/fs_contract.cpp"
  "${XV_ROOT}/tests/fs_test_platform.cpp"
  "${XV_ROOT}/tests/fs_driver_platform.cpp"
  "${XV_ROOT}/kernel/fs/procfs/procfs.cpp"
  "${XV_ROOT}/kernel/fs/vfs/vfs.cpp"
  "${XV_ROOT}/kernel/fs/devfs/devfs.cpp"
  "${XV_ROOT}/tests/sync_test_platform.cpp"
  "${XV_ROOT}/kernel/sync/spin/spinlock.cpp"
  "${XV_ROOT}/kernel/sync/sleep/sleeplock.cpp"
  "${XV_ROOT}/kernel/fs/cache/lru_block_cache.cpp"
  "${XV_ROOT}/kernel/fs/journal/journal.cpp"
  "${XV_ROOT}/kernel/fs/inode/xv6_filesystem.cpp"
  "${XV_ROOT}/kernel/fs/path/path_operations.cpp"
  "${XV_ROOT}/kernel/fs/file/open_file.cpp"
  "${XV_ROOT}/kernel/fs/file/file_operations.cpp"
  "${XV_ROOT}/kernel/fs/pipe/pipe.cpp"
  "${XV_ROOT}/kernel/fs/pipe/pipe_factory.cpp")
target_include_directories(fs_contract PRIVATE "${XV_ROOT}")
target_compile_features(fs_contract PRIVATE cxx_std_20)
target_compile_options(fs_contract PRIVATE -O2 -Wall -Wextra -Werror -Wno-sign-compare -fno-rtti -fno-exceptions)
target_link_libraries(fs_contract PRIVATE Threads::Threads)
set_target_properties(fs_contract PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${XV_OUTPUT_DIR}")
xv_add_sync_modules(fs_contract)
xv_add_driver_interfaces(fs_contract)
xv_add_fs_interfaces(fs_contract)
xv_add_fs_internals(fs_contract)

include("${XV_ROOT}/cmake/trap-modules.cmake")
add_executable(trap_contract
  "${XV_ROOT}/tests/trap_contract.cpp"
  "${XV_ROOT}/tests/trap_test_platform.cpp"
  "${XV_ROOT}/tests/sync_test_platform.cpp"
  "${XV_ROOT}/kernel/sync/spin/spinlock.cpp"
  "${XV_ROOT}/kernel/sync/sleep/sleeplock.cpp"
  "${XV_ROOT}/kernel/trap/arch/riscv/entry.cpp"
  "${XV_ROOT}/kernel/trap/dispatch/trap_dispatcher.cpp"
  "${XV_ROOT}/kernel/trap/controller/trap_controller.cpp"
  "${XV_ROOT}/kernel/trap/clock/tick_clock.cpp"
  "${XV_ROOT}/kernel/trap/handlers/system_call.cpp"
  "${XV_ROOT}/kernel/trap/handlers/external_interrupt.cpp"
  "${XV_ROOT}/kernel/trap/handlers/timer_interrupt.cpp"
  "${XV_ROOT}/kernel/trap/handlers/page_fault.cpp"
  "${XV_ROOT}/kernel/trap/service.cpp")
target_include_directories(trap_contract PRIVATE "${XV_ROOT}")
target_compile_features(trap_contract PRIVATE cxx_std_20)
target_compile_options(trap_contract PRIVATE -O2 -Wall -Wextra -Werror -fno-rtti -fno-exceptions)
target_link_libraries(trap_contract PRIVATE Threads::Threads)
set_target_properties(trap_contract PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${XV_OUTPUT_DIR}")
xv_add_sync_modules(trap_contract)
xv_add_trap_modules(trap_contract)

include("${XV_ROOT}/cmake/runtime-modules.cmake")
add_executable(runtime_contract
  "${XV_ROOT}/tests/runtime_contract.cpp"
  "${XV_ROOT}/kernel/support/string.cpp")
target_include_directories(runtime_contract PRIVATE "${XV_ROOT}")
target_compile_features(runtime_contract PRIVATE cxx_std_20)
target_compile_options(runtime_contract PRIVATE -O2 -Wall -Wextra -Werror -fno-builtin)
set_target_properties(runtime_contract PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${XV_OUTPUT_DIR}")
xv_add_runtime_module(runtime_contract)

add_executable(process_storage_contract
  "${XV_ROOT}/tests/process_storage_contract.cpp"
  "${XV_ROOT}/tests/process_storage_checks.cpp"
  "${XV_ROOT}/tests/process_storage_test_platform.cpp"
  "${XV_ROOT}/tests/sync_test_platform.cpp"
  "${XV_ROOT}/kernel/process/storage/process_store.cpp"
  "${XV_ROOT}/kernel/process/signals/process_signals.cpp"
  "${XV_ROOT}/kernel/process/storage/paged_process_store.cpp"
  "${XV_ROOT}/kernel/process/wait/wait_channels.cpp"
  "${XV_ROOT}/kernel/process/wait/poll_wait.cpp"
  "${XV_ROOT}/kernel/process/scheduler/queues/ready_queue.cpp"
  "${XV_ROOT}/kernel/process/scheduler/queues/fifo_ready_queue.cpp"
  "${XV_ROOT}/kernel/process/arch/cpu_affinity.cpp"
  "${XV_ROOT}/kernel/sync/spin/spinlock.cpp"
  "${XV_ROOT}/kernel/sync/sleep/sleeplock.cpp")
target_include_directories(process_storage_contract PRIVATE "${XV_ROOT}")
target_compile_features(process_storage_contract PRIVATE cxx_std_20)
target_compile_options(process_storage_contract PRIVATE -O2 -Wall -Wextra -Werror -fno-rtti -fno-exceptions)
target_link_libraries(process_storage_contract PRIVATE Threads::Threads)
set_target_properties(process_storage_contract PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${XV_OUTPUT_DIR}")
xv_add_sync_modules(process_storage_contract)
xv_add_driver_interfaces(process_storage_contract)
xv_add_fs_interfaces(process_storage_contract)
xv_add_process_interfaces(process_storage_contract)
xv_add_process_core_modules(process_storage_contract)

add_executable(process_groups_contract
  "${XV_ROOT}/tests/process_groups_contract.cpp"
  "${XV_ROOT}/tests/process_groups_checks.cpp"
  "${XV_ROOT}/tests/signal_checks.cpp"
  "${XV_ROOT}/tests/sync_test_platform.cpp"
  "${XV_ROOT}/kernel/process/groups/process_groups.cpp"
  "${XV_ROOT}/kernel/process/groups/terminal_control.cpp"
  "${XV_ROOT}/kernel/sync/spin/spinlock.cpp"
  "${XV_ROOT}/kernel/sync/sleep/sleeplock.cpp")
target_include_directories(process_groups_contract PRIVATE "${XV_ROOT}")
target_compile_features(process_groups_contract PRIVATE cxx_std_20)
target_compile_options(process_groups_contract PRIVATE -O2 -Wall -Wextra -Werror -fno-rtti -fno-exceptions)
target_link_libraries(process_groups_contract PRIVATE Threads::Threads)
set_target_properties(process_groups_contract PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${XV_OUTPUT_DIR}")
xv_add_sync_modules(process_groups_contract)
xv_add_driver_interfaces(process_groups_contract)
xv_add_fs_interfaces(process_groups_contract)
xv_add_process_interfaces(process_groups_contract)
xv_add_process_group_internals(process_groups_contract)

add_executable(network_contract
  "${XV_ROOT}/tests/network_contract.cpp"
  "${XV_ROOT}/tests/tcp_contract.cpp"
  "${XV_ROOT}/tests/tcp_buffers.cpp"
  "${XV_ROOT}/tests/network_interfaces.cpp"
  "${XV_ROOT}/tests/network_control.cpp"
  "${XV_ROOT}/tests/bridge_contract.cpp"
  "${XV_ROOT}/tests/dhcp_contract.cpp"
  "${XV_ROOT}/tests/network_test_device.cpp"
  "${XV_ROOT}/tests/network_test_platform.cpp"
  "${XV_ROOT}/tests/sync_test_platform.cpp"
  "${XV_ROOT}/kernel/sync/spin/spinlock.cpp"
  "${XV_ROOT}/kernel/sync/sleep/sleeplock.cpp"
)
include("${XV_ROOT}/cmake/net-modules.cmake")
xv_add_net_modules(network_contract)
target_include_directories(network_contract PRIVATE "${XV_ROOT}")
target_compile_features(network_contract PRIVATE cxx_std_20)
target_compile_options(network_contract PRIVATE -O2 -Wall -Wextra -Werror -fno-rtti -fno-exceptions)
target_link_libraries(network_contract PRIVATE Threads::Threads)
set_target_properties(network_contract PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${XV_OUTPUT_DIR}")
xv_add_sync_modules(network_contract)
xv_add_driver_interfaces(network_contract)

add_executable(dns_contract "${XV_ROOT}/tests/dns_contract.cpp" "${XV_ROOT}/user/runtime/dns/codec.cpp")
target_include_directories(dns_contract PRIVATE "${XV_ROOT}")
target_compile_features(dns_contract PRIVATE cxx_std_20)
target_compile_options(dns_contract PRIVATE -O2 -Wall -Wextra -Werror -UNDEBUG)
set_target_properties(dns_contract PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${XV_OUTPUT_DIR}")

add_executable(http_contract "${XV_ROOT}/tests/http_contract.cpp" "${XV_ROOT}/user/runtime/http/codec.cpp")
target_include_directories(http_contract PRIVATE "${XV_ROOT}")
target_compile_features(http_contract PRIVATE cxx_std_20)
target_compile_options(http_contract PRIVATE -O2 -Wall -Wextra -Werror -UNDEBUG)
set_target_properties(http_contract PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${XV_OUTPUT_DIR}")
