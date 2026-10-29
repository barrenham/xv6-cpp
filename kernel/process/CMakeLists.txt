include("${PROJECT_SOURCE_DIR}/cmake/process-modules.cmake")
xv_add_process_interfaces(kernel)
xv_add_process_internals(kernel)
xv_add_address_space_flow(kernel)

target_sources(kernel PRIVATE
  arch/cpu.cpp
  lifecycle/first_return.cpp
  wait/poll_wait.cpp
  wait/sleep.cpp
  wait/wait_child.cpp
  scheduler/runtime/scheduler_context.cpp
  lifecycle/process.cpp
  signals/process_signals.cpp
  signals/user_signals.cpp
  signals/signal_delivery.cpp
  lifecycle/process_manager.cpp
  groups/process_groups.cpp
  groups/kernel_platform.cpp
  groups/group_operations.cpp
  groups/terminal_control.cpp
  storage/process_store.cpp
  storage/paged_process_store.cpp
  storage/kernel_platform.cpp
  wait/wait_channels.cpp
  scheduler/queues/ready_queue.cpp
  scheduler/queues/policy_ready_queue.cpp
  scheduler/queues/fifo_ready_queue.cpp
  arch/cpu_affinity.cpp
  lifecycle/service.cpp
  resources/process_resources.cpp
  resources/kernel_platform.cpp
  memory/address_space.cpp
  scheduler/runtime/scheduler_runtime.cpp
  exec/elf_loader.cpp
  arch/swtch.S
)

set(XV_SCHEDULER "fair" CACHE STRING "Boot scheduler: fair, round-robin or mlfq")
set_property(CACHE XV_SCHEDULER PROPERTY STRINGS fair round-robin mlfq)
if(XV_SCHEDULER STREQUAL "fair")
  target_compile_definitions(kernel PRIVATE XV_DEFAULT_SCHEDULER=0)
elseif(XV_SCHEDULER STREQUAL "round-robin")
  target_compile_definitions(kernel PRIVATE XV_DEFAULT_SCHEDULER=1)
elseif(XV_SCHEDULER STREQUAL "mlfq")
  target_compile_definitions(kernel PRIVATE XV_DEFAULT_SCHEDULER=2)
else()
  message(FATAL_ERROR "XV_SCHEDULER must be fair, round-robin or mlfq")
endif()
