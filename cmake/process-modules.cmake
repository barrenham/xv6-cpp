function(xv_add_process_interfaces target)
  set(module_root "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../kernel/process")
  target_sources(${target} PRIVATE
    FILE_SET process_interfaces TYPE CXX_MODULES
    BASE_DIRS "${module_root}"
    FILES
      "${module_root}/process_module.cppm"
      "${module_root}/arch/registers.cppm"
      "${module_root}/resources/process_resources.cppm"
      "${module_root}/resources/resource_platform.cppm"
      "${module_root}/memory/address_space.cppm"
      "${module_root}/process.cppm"
      "${module_root}/scheduler/scheduler.cppm"
      "${module_root}/exec/executable_loader.cppm"
  )
  set_property(TARGET ${target} PROPERTY CXX_SCAN_FOR_MODULES ON)
endfunction()

function(xv_add_process_core_modules target)
  set(module_root "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../kernel/process")
  target_sources(${target} PRIVATE
    FILE_SET process_core_modules TYPE CXX_MODULES
    BASE_DIRS "${module_root}"
    FILES
      "${module_root}/arch/cpu.cppm"
      "${module_root}/storage/process_store.cppm"
      "${module_root}/storage/paged_process_store.cppm"
      "${module_root}/storage/platform.cppm"
      "${module_root}/scheduler/queues/ready_queue.cppm"
      "${module_root}/wait/wait_channels.cppm"
      "${module_root}/scheduler/queues/fifo_ready_queue.cppm"
  )
endfunction()

function(xv_add_process_internals target)
  xv_add_process_core_modules(${target})
  set(module_root "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../kernel/process")
  target_sources(${target} PRIVATE
    FILE_SET process_internals TYPE CXX_MODULES
    BASE_DIRS "${module_root}"
    FILES
      "${module_root}/lifecycle/process_manager.cppm"
      "${module_root}/scheduler/policies/round_robin.cppm"
      "${module_root}/scheduler/runtime/scheduler_context.cppm"
      "${module_root}/exec/elf_format.cppm"
      "${module_root}/exec/elf_loader.cppm"
  )
endfunction()
