cmake_minimum_required(VERSION 3.28)
project(xv_kernel_driver NONE)
find_program(XVK_CXX clang++ REQUIRED)
find_program(XVK_LD ld.lld REQUIRED)
set(XVK_ABI_DIR "${CMAKE_CURRENT_LIST_DIR}/../../kernel/io" CACHE PATH "Directory containing kmodule.h")
set(XVK_SOURCE "${CMAKE_CURRENT_LIST_DIR}/virtio_block.cpp" CACHE FILEPATH "Independent module source")
set(XVK_NAME "virtio_block" CACHE STRING "Module identity and output filename")
string(LENGTH "${XVK_NAME}" XVK_NAME_LENGTH)
if(NOT XVK_NAME MATCHES "^[A-Za-z0-9_-]+$" OR XVK_NAME_LENGTH GREATER 31)
  message(FATAL_ERROR "Invalid module name")
endif()
add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${XVK_NAME}.o"
  COMMAND "${XVK_CXX}" --no-default-config --target=riscv64-unknown-elf
    -march=rv64imac -mabi=lp64 -mcmodel=medany -mno-relax -msmall-data-limit=0
    -O2 -g -std=c++20 -ffreestanding -fPIC -fvisibility=hidden
    -fno-builtin -fno-stack-protector -fno-exceptions -fno-rtti
    -fno-threadsafe-statics -fno-use-cxa-atexit -nostdinc++
    "-DXVK_MODULE_NAME=\"${XVK_NAME}\"" "-I${XVK_ABI_DIR}"
    -c "${XVK_SOURCE}" -o "${CMAKE_CURRENT_BINARY_DIR}/${XVK_NAME}.o"
  DEPENDS "${XVK_SOURCE}" "${XVK_ABI_DIR}/kmodule.h" VERBATIM)
add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${XVK_NAME}.elf"
  COMMAND "${XVK_LD}" -shared -Bsymbolic --no-undefined --no-relax
    -z max-page-size=4096 -z separate-code -e xv_driver_entry
    "${CMAKE_CURRENT_BINARY_DIR}/${XVK_NAME}.o"
    -o "${CMAKE_CURRENT_BINARY_DIR}/${XVK_NAME}.elf"
  DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${XVK_NAME}.o" VERBATIM)
add_custom_target(driver ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${XVK_NAME}.elf")
