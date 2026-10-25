cmake_minimum_required(VERSION 3.28)
project(xv_labs_cpp_2026 LANGUAGES CXX ASM)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(XV_RAM_MB 128 CACHE STRING "QEMU RAM size in MiB (kernel discovers it from DTB)")
if(NOT XV_RAM_MB MATCHES "^[1-9][0-9]*$")
  message(FATAL_ERROR "XV_RAM_MB must be a positive integer")
endif()
set(XV_CPUS 3 CACHE STRING "Number of QEMU harts")
set(XV_GDB_PORT 26000 CACHE STRING "QEMU GDB port")
find_program(XV_LLD ld.lld HINTS "$ENV{CONDA_PREFIX}/bin" REQUIRED)
find_program(XV_HOST_CLANG clang++ HINTS "$ENV{CONDA_PREFIX}/bin" REQUIRED)
find_program(XV_QEMU qemu-system-riscv64 HINTS "$ENV{CONDA_PREFIX}/bin" REQUIRED)
find_package(Python3 REQUIRED COMPONENTS Interpreter)
add_library(xv_options INTERFACE)
target_include_directories(xv_options INTERFACE "${PROJECT_SOURCE_DIR}")
target_compile_definitions(xv_options INTERFACE LAB_PGTBL SOL_PGTBL)
target_compile_options(xv_options INTERFACE
  -march=rv64gc -mabi=lp64d -mcmodel=medany -mno-relax
  -ffreestanding -fno-builtin -fno-stack-protector -fno-pic -fno-pie
  -fno-omit-frame-pointer -g -O1
  "$<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions;-fno-rtti;-fno-threadsafe-statics;-fno-use-cxa-atexit;-fno-unwind-tables;-fno-asynchronous-unwind-tables;-nostdinc++;-Wall;-Wextra;-Werror;-Wno-write-strings;-Wno-writable-strings;-Wno-unused-parameter;-Wno-sign-compare;-Wno-missing-field-initializers>"
)
# Direct LLD linkage makes host libraries and C++ startup objects impossible to pull in.
set(CMAKE_CXX_LINK_EXECUTABLE "${XV_LLD} <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
add_subdirectory(kernel)
add_subdirectory(user)
add_subdirectory(tools/mkfs)
configure_file(README.xv6 "${PROJECT_BINARY_DIR}/README" COPYONLY)
add_custom_command(OUTPUT "${PROJECT_BINARY_DIR}/fs.img"
  COMMAND "${PROJECT_BINARY_DIR}/mkfs" "${PROJECT_BINARY_DIR}/fs.img" "${PROJECT_BINARY_DIR}/README" ${XV_USER_BINARIES}
  DEPENDS host_mkfs "${PROJECT_BINARY_DIR}/mkfs" "${PROJECT_BINARY_DIR}/README" ${XV_USER_BINARIES}
  VERBATIM)
add_custom_target(image ALL DEPENDS "${PROJECT_BINARY_DIR}/fs.img")
set(XV_QEMU_OPTIONS -machine virt -bios none -kernel $<TARGET_FILE:kernel> -m ${XV_RAM_MB}M -smp ${XV_CPUS} -nographic -global virtio-mmio.force-legacy=false -drive file=${PROJECT_BINARY_DIR}/fs.img,if=none,format=raw,id=x0 -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0)
add_custom_target(qemu COMMAND "${XV_QEMU}" ${XV_QEMU_OPTIONS} DEPENDS kernel image USES_TERMINAL)
add_custom_target(qemu-gdb COMMAND "${XV_QEMU}" ${XV_QEMU_OPTIONS} -S -gdb tcp::${XV_GDB_PORT} DEPENDS kernel image USES_TERMINAL)
enable_testing()
add_test(NAME runtime_contract COMMAND "${PROJECT_BINARY_DIR}/runtime_contract")
set_tests_properties(runtime_contract PROPERTIES TIMEOUT 10 LABELS runtime)
add_test(NAME trap_contract COMMAND "${PROJECT_BINARY_DIR}/trap_contract")
set_tests_properties(trap_contract PROPERTIES TIMEOUT 45 LABELS trap)
add_test(NAME fs_contract COMMAND "${PROJECT_BINARY_DIR}/fs_contract" "${PROJECT_BINARY_DIR}/fs.img")
set_tests_properties(fs_contract PROPERTIES TIMEOUT 45 LABELS fs)
add_test(NAME process_contract COMMAND "${PROJECT_BINARY_DIR}/process_contract")
set_tests_properties(process_contract PROPERTIES TIMEOUT 45 LABELS process)
add_test(NAME driver_contract COMMAND "${PROJECT_BINARY_DIR}/driver_contract")
set_tests_properties(driver_contract PROPERTIES TIMEOUT 45 LABELS drivers)
add_test(NAME sync_contract COMMAND "${PROJECT_BINARY_DIR}/sync_contract")
set_tests_properties(sync_contract PROPERTIES TIMEOUT 45 LABELS sync)
add_test(NAME boot_smoke COMMAND "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tests/smoke.py" --qemu "${XV_QEMU}" --kernel $<TARGET_FILE:kernel> --image "${PROJECT_BINARY_DIR}/fs.img" --allocator "${XV_PAGE_ALLOCATOR}" --ram-mib "${XV_RAM_MB}")
set_tests_properties(boot_smoke PROPERTIES TIMEOUT 120)
add_test(NAME user_regression COMMAND "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tests/smoke.py" --qemu "${XV_QEMU}" --kernel $<TARGET_FILE:kernel> --image "${PROJECT_BINARY_DIR}/fs.img" --allocator "${XV_PAGE_ALLOCATOR}" --ram-mib "${XV_RAM_MB}" --regression)
set_tests_properties(user_regression PROPERTIES TIMEOUT 270)
foreach(kind buddy freelist)
  add_test(NAME allocator_contract_${kind} COMMAND "${PROJECT_BINARY_DIR}/allocator_contract" ${kind})
  set_tests_properties(allocator_contract_${kind} PROPERTIES TIMEOUT 45 LABELS memory)
endforeach()

# Same kernel binary, different firmware-described RAM lengths and mapping edges.
foreach(ram 64 257 2048 2049)
  add_test(NAME ram_${ram}MiB COMMAND "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tests/smoke.py"
    --qemu "${XV_QEMU}" --kernel $<TARGET_FILE:kernel> --image "${PROJECT_BINARY_DIR}/fs.img"
    --allocator "${XV_PAGE_ALLOCATOR}" --ram-mib ${ram} --boot-only)
  set_tests_properties(ram_${ram}MiB PROPERTIES TIMEOUT 120 LABELS ram)
endforeach()
