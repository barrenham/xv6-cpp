function(xv_add_sync_modules target)
  set(module_root "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../kernel/sync")
  target_sources(${target} PRIVATE
    FILE_SET sync_modules TYPE CXX_MODULES
    BASE_DIRS "${module_root}"
    FILES
      "${module_root}/sync.cppm"
      "${module_root}/interrupts.cppm"
      "${module_root}/lock.cppm"
      "${module_root}/spin/spinlock.cppm"
      "${module_root}/sleep/sleeplock.cppm"
      "${module_root}/scoped_lock.cppm"
      "${module_root}/platform/platform.cppm"
  )
  set_property(TARGET ${target} PROPERTY CXX_SCAN_FOR_MODULES ON)
endfunction()
