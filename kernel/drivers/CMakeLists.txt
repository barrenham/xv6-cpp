include("${PROJECT_SOURCE_DIR}/cmake/driver-modules.cmake")
xv_add_driver_modules(kernel)

target_sources(kernel PRIVATE
  FILE_SET pty_internal TYPE CXX_MODULES BASE_DIRS "${PROJECT_SOURCE_DIR}"
  FILES tty/pty/pty.cppm
  modules/runtime_internal.cppm modules/serial_proxy.cppm
)

target_sources(kernel PRIVATE
  tty/pty/pty.cpp
  tty/core/tty.cpp
  tty/ldisc/n_tty.cpp
  interrupt/plic.cpp
  interrupt/interrupt_router.cpp
  platform/kernel_platform.cpp
  board/qemu_virt.cpp
  modules/runtime.cpp
  modules/resources.cpp
  modules/services.cpp
)
