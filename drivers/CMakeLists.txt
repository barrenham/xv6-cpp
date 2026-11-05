# Device ELF targets use the integer RV64 ABI and their own PIC objects. They
# never link kernel objects or resolve kernel-private symbols.
find_program(XV_DRIVER_OBJCOPY llvm-objcopy HINTS "$ENV{CONDA_PREFIX}/bin" REQUIRED)
add_library(driver_options INTERFACE)
target_include_directories(driver_options INTERFACE "${PROJECT_SOURCE_DIR}")
target_compile_options(driver_options INTERFACE
  -march=rv64imac -mabi=lp64 -mcmodel=medany -mno-relax -msmall-data-limit=0
  -ffreestanding -fno-builtin -fno-stack-protector -fPIC -fvisibility=hidden
  -O2 -g -fno-exceptions -fno-rtti -fno-threadsafe-statics -fno-use-cxa-atexit
  -fno-unwind-tables -fno-asynchronous-unwind-tables -nostdinc++)
target_compile_definitions(driver_options INTERFACE
  XV_DISPLAY_WIDTH=${XV_DISPLAY_WIDTH} XV_DISPLAY_HEIGHT=${XV_DISPLAY_HEIGHT})

add_library(device_sdk STATIC
  sdk/host_state.cpp sdk/runtime.cpp sdk/driver_platform.cpp sdk/sync_platform.cpp
  "${PROJECT_SOURCE_DIR}/kernel/sync/spin/spinlock.cpp"
  "${PROJECT_SOURCE_DIR}/kernel/sync/sleep/sleeplock.cpp")
target_link_libraries(device_sdk PUBLIC driver_options)
set_property(TARGET device_sdk PROPERTY CXX_SCAN_FOR_MODULES ON)
set(driver_interface_root "${PROJECT_SOURCE_DIR}/kernel/drivers")
set(sync_interface_root "${PROJECT_SOURCE_DIR}/kernel/sync")
target_sources(device_sdk PUBLIC FILE_SET sdk_modules TYPE CXX_MODULES
  BASE_DIRS "${PROJECT_SOURCE_DIR}"
  FILES
    "${driver_interface_root}/drivers.cppm"
    "${driver_interface_root}/device.cppm"
    "${driver_interface_root}/serial/serial_port.cppm"
    "${driver_interface_root}/tty/core/terminal_output.cppm"
    "${driver_interface_root}/console/character_device.cppm"
    "${driver_interface_root}/block/block_device.cppm"
    "${driver_interface_root}/net/ethernet.cppm"
    "${driver_interface_root}/interrupt/interrupt_controller.cppm"
    "${driver_interface_root}/interrupt/interrupt_router.cppm"
    "${driver_interface_root}/tty/ldisc/line_discipline.cppm"
    "${driver_interface_root}/platform/platform.cppm"
    "${CMAKE_CURRENT_SOURCE_DIR}/sdk/virtio.cppm"
    "${driver_interface_root}/mmio.cppm"
    "${sync_interface_root}/sync.cppm"
    "${sync_interface_root}/lock.cppm"
    "${sync_interface_root}/spin/spinlock.cppm"
    "${sync_interface_root}/sleep/sleeplock.cppm"
    "${sync_interface_root}/sleep/flow.cppm"
    "${sync_interface_root}/scoped_lock.cppm"
    "${sync_interface_root}/interrupts.cppm"
    "${sync_interface_root}/poll_table.cppm"
    "${sync_interface_root}/platform/platform.cppm")

function(xv_add_device_elf name)
  add_executable(driver_${name} ${ARGN})
  target_link_libraries(driver_${name} PRIVATE device_sdk)
  target_link_options(driver_${name} PRIVATE
    -shared -Bsymbolic --no-undefined --no-relax
    "SHELL:-z max-page-size=4096" "SHELL:-z separate-code"
    -e xv_driver_entry)
  set_target_properties(driver_${name} PROPERTIES CXX_SCAN_FOR_MODULES ON
    OUTPUT_NAME "${name}.elf" RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/drivers")
  add_custom_command(TARGET driver_${name} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy $<TARGET_FILE:driver_${name}>
      "${PROJECT_BINARY_DIR}/drivers/${name}.debug.elf"
    COMMAND "${XV_DRIVER_OBJCOPY}" --strip-debug $<TARGET_FILE:driver_${name}>
    BYPRODUCTS "${PROJECT_BINARY_DIR}/drivers/${name}.debug.elf" VERBATIM)
endfunction()

xv_add_device_elf(rng backends/entropy/main.cpp backends/entropy/device.cpp)
target_sources(driver_rng PRIVATE FILE_SET rng_module TYPE CXX_MODULES
  FILES backends/entropy/device.cppm)
xv_add_device_elf(mem backends/memory/main.cpp)
xv_add_device_elf(gpu backends/gpu/main.cpp backends/gpu/device.cpp)
target_sources(driver_gpu PRIVATE FILE_SET gpu_module TYPE CXX_MODULES
  FILES backends/gpu/device.cppm)
xv_add_device_elf(input backends/input/main.cpp backends/input/device.cpp)
target_sources(driver_input PRIVATE FILE_SET input_module TYPE CXX_MODULES
  FILES backends/input/device.cppm)
xv_add_device_elf(serial backends/serial/main.cpp backends/serial/device.cpp)
target_sources(driver_serial PRIVATE FILE_SET serial_module TYPE CXX_MODULES
  FILES backends/serial/device.cppm backends/serial/flow.cppm)
xv_add_device_elf(e1000 backends/e1000/main.cpp backends/e1000/device.cpp)
target_sources(driver_e1000 PRIVATE FILE_SET e1000_module TYPE CXX_MODULES
  FILES backends/e1000/device.cppm)
