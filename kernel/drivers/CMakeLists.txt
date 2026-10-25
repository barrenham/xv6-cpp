include("${PROJECT_SOURCE_DIR}/cmake/driver-modules.cmake")
xv_add_driver_modules(kernel)

target_sources(kernel PRIVATE
  serial/uart16550.cpp
  tty/tty.cpp
  tty/n_tty.cpp
  block/virtio/virtio_block.cpp
  interrupt/plic.cpp
  interrupt/interrupt_router.cpp
  platform/kernel_platform.cpp
  board/qemu_virt.cpp
)
