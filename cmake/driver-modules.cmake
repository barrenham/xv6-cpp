function(xv_add_driver_modules target)
  set(module_root "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../kernel/drivers")
  target_sources(${target} PRIVATE
    FILE_SET driver_modules TYPE CXX_MODULES
    BASE_DIRS "${module_root}"
    FILES
      "${module_root}/drivers.cppm"
      "${module_root}/device.cppm"
      "${module_root}/serial/serial_port.cppm"
      "${module_root}/serial/uart16550.cppm"
      "${module_root}/console/character_device.cppm"
      "${module_root}/console/line_console.cppm"
      "${module_root}/block/block_device.cppm"
      "${module_root}/block/virtio/virtio_block.cppm"
      "${module_root}/block/virtio/protocol.cppm"
      "${module_root}/interrupt/interrupt_controller.cppm"
      "${module_root}/interrupt/interrupt_router.cppm"
      "${module_root}/interrupt/plic.cppm"
      "${module_root}/platform/platform.cppm"
      "${module_root}/mmio.cppm"
  )
  set_property(TARGET ${target} PROPERTY CXX_SCAN_FOR_MODULES ON)
endfunction()
