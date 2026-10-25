include("${PROJECT_SOURCE_DIR}/cmake/trap-modules.cmake")
xv_add_trap_modules(kernel)
target_sources(kernel PRIVATE
  arch/riscv/kernelvec.S
  arch/riscv/trampoline.S
  arch/riscv/entry.cpp
  arch/riscv/kernel_platform.cpp
  dispatch/trap_dispatcher.cpp
  controller/trap_controller.cpp
  clock/tick_clock.cpp
  handlers/system_call.cpp
  handlers/external_interrupt.cpp
  handlers/timer_interrupt.cpp
  handlers/page_fault.cpp
  service.cpp
)
