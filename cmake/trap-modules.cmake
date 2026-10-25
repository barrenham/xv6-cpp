function(xv_add_trap_modules target)
  set(module_root "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../kernel/trap")
  target_sources(${target} PRIVATE
    FILE_SET trap_modules TYPE CXX_MODULES
    BASE_DIRS "${module_root}"
    FILES
      "${module_root}/trap.cppm"
      "${module_root}/event/trap_event.cppm"
      "${module_root}/dispatch/trap_dispatcher.cppm"
      "${module_root}/clock/tick_clock.cppm"
      "${module_root}/controller/trap_controller.cppm"
      "${module_root}/handlers/handlers.cppm"
      "${module_root}/platform/platform.cppm"
  )
  set_property(TARGET ${target} PROPERTY CXX_SCAN_FOR_MODULES ON)
endfunction()
