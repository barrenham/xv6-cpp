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
target_compile_definitions(xv_options INTERFACE LAB_PGTBL SOL_PGTBL LAB_NET)
target_compile_options(xv_options INTERFACE
  -march=rv64gc -mabi=lp64d -mcmodel=medany -mno-relax
  -ffreestanding -fno-builtin -fno-stack-protector -fno-pic -fno-pie
  -fno-omit-frame-pointer -g -O1
  "$<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions;-fno-rtti;-fno-threadsafe-statics;-fno-use-cxa-atexit;-fno-unwind-tables;-fno-asynchronous-unwind-tables;-nostdinc++;-Wall;-Wextra;-Werror;-Wno-write-strings;-Wno-writable-strings;-Wno-unused-parameter;-Wno-sign-compare;-Wno-missing-field-initializers>"
)
# Direct LLD linkage makes host libraries and C++ startup objects impossible to pull in.
set(CMAKE_CXX_LINK_EXECUTABLE "${XV_LLD} <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
option(XV_NET_DHCP "Acquire network addresses with DHCP at boot" ON)
add_subdirectory(kernel)
target_compile_definitions(kernel PRIVATE XV_NET_BOOT_DHCP=$<BOOL:${XV_NET_DHCP}>)
add_subdirectory(user)
add_subdirectory(tools/mkfs)
configure_file(README.xv6 "${PROJECT_BINARY_DIR}/README" COPYONLY)
add_custom_command(OUTPUT "${PROJECT_BINARY_DIR}/fs.img"
  COMMAND "${PROJECT_BINARY_DIR}/mkfs" "${PROJECT_BINARY_DIR}/fs.img" "${PROJECT_BINARY_DIR}/README" ${XV_USER_BINARIES}
  DEPENDS host_mkfs "${PROJECT_BINARY_DIR}/mkfs" "${PROJECT_BINARY_DIR}/README" ${XV_USER_BINARIES}
  VERBATIM)
add_custom_target(image ALL DEPENDS "${PROJECT_BINARY_DIR}/fs.img")
set(XV_QEMU_OPTIONS -machine virt -bios none -kernel $<TARGET_FILE:kernel> -m ${XV_RAM_MB}M -smp ${XV_CPUS} -nographic -global virtio-mmio.force-legacy=false -drive file=${PROJECT_BINARY_DIR}/fs.img,if=none,format=raw,id=x0 -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0)
set(XV_NET_BACKEND "user,id=net0" CACHE STRING "QEMU network backend; append hostfwd for inbound UDP")
list(APPEND XV_QEMU_OPTIONS -netdev ${XV_NET_BACKEND} -device e1000,netdev=net0,bus=pcie.0,addr=01.0,mac=52:54:00:12:34:56)
add_custom_target(qemu COMMAND "${XV_QEMU}" ${XV_QEMU_OPTIONS} DEPENDS kernel image USES_TERMINAL)
add_custom_target(qemu-gdb COMMAND "${XV_QEMU}" ${XV_QEMU_OPTIONS} -S -gdb tcp::${XV_GDB_PORT} DEPENDS kernel image USES_TERMINAL)
enable_testing()
add_test(NAME signal_contract COMMAND "${PROJECT_BINARY_DIR}/process_groups_contract" --signals)
set_tests_properties(signal_contract PROPERTIES TIMEOUT 45 LABELS process)
add_test(NAME process_groups_contract COMMAND "${PROJECT_BINARY_DIR}/process_groups_contract")
set_tests_properties(process_groups_contract PROPERTIES TIMEOUT 45 LABELS process)
add_test(NAME process_storage_contract COMMAND "${PROJECT_BINARY_DIR}/process_storage_contract")
set_tests_properties(process_storage_contract PROPERTIES TIMEOUT 45 LABELS process)
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

foreach(cpus 1 3)
  foreach(feature named-pty procfs)
    add_test(NAME ${feature}_${cpus}cpu COMMAND "${Python3_EXECUTABLE}"
      "${PROJECT_SOURCE_DIR}/tests/smoke.py" --qemu "${XV_QEMU}"
      --kernel $<TARGET_FILE:kernel> --image "${PROJECT_BINARY_DIR}/fs.img"
      --allocator "${XV_PAGE_ALLOCATOR}" --ram-mib "${XV_RAM_MB}"
      --${feature} --cpus ${cpus})
    set_tests_properties(${feature}_${cpus}cpu PROPERTIES TIMEOUT 45 LABELS fs)
  endforeach()

  add_test(NAME devfs_${cpus}cpu COMMAND "${Python3_EXECUTABLE}"
    "${PROJECT_SOURCE_DIR}/tests/smoke.py" --qemu "${XV_QEMU}"
    --kernel $<TARGET_FILE:kernel> --image "${PROJECT_BINARY_DIR}/fs.img"
    --allocator "${XV_PAGE_ALLOCATOR}" --ram-mib "${XV_RAM_MB}"
    --devfs --cpus ${cpus})
  set_tests_properties(devfs_${cpus}cpu PROPERTIES TIMEOUT 40 LABELS fs)
  add_test(NAME sigchld_${cpus}cpu COMMAND "${Python3_EXECUTABLE}"
    "${PROJECT_SOURCE_DIR}/tests/smoke.py" --qemu "${XV_QEMU}"
    --kernel $<TARGET_FILE:kernel> --image "${PROJECT_BINARY_DIR}/fs.img"
    --allocator "${XV_PAGE_ALLOCATOR}" --ram-mib "${XV_RAM_MB}"
    --sigchld --cpus ${cpus})
  set_tests_properties(sigchld_${cpus}cpu PROPERTIES TIMEOUT 30 LABELS process)
  add_test(NAME nonblock_${cpus}cpu COMMAND "${Python3_EXECUTABLE}"
    "${PROJECT_SOURCE_DIR}/tests/smoke.py" --qemu "${XV_QEMU}"
    --kernel $<TARGET_FILE:kernel> --image "${PROJECT_BINARY_DIR}/fs.img"
    --allocator "${XV_PAGE_ALLOCATOR}" --ram-mib "${XV_RAM_MB}"
    --nonblock --cpus ${cpus})
  set_tests_properties(nonblock_${cpus}cpu PROPERTIES TIMEOUT 20 LABELS io)
  add_test(NAME poll_${cpus}cpu COMMAND "${Python3_EXECUTABLE}"
    "${PROJECT_SOURCE_DIR}/tests/smoke.py" --qemu "${XV_QEMU}"
    --kernel $<TARGET_FILE:kernel> --image "${PROJECT_BINARY_DIR}/fs.img"
    --allocator "${XV_PAGE_ALLOCATOR}" --ram-mib "${XV_RAM_MB}"
    --poll --cpus ${cpus})
  set_tests_properties(poll_${cpus}cpu PROPERTIES TIMEOUT 15 LABELS io)
  add_test(NAME pty_${cpus}cpu COMMAND "${Python3_EXECUTABLE}"
    "${PROJECT_SOURCE_DIR}/tests/smoke.py" --qemu "${XV_QEMU}"
    --kernel $<TARGET_FILE:kernel> --image "${PROJECT_BINARY_DIR}/fs.img"
    --allocator "${XV_PAGE_ALLOCATOR}" --ram-mib "${XV_RAM_MB}"
    --pty --cpus ${cpus})
  set_tests_properties(pty_${cpus}cpu PROPERTIES TIMEOUT 35 LABELS tty)
  add_test(NAME jobs_${cpus}cpu COMMAND "${Python3_EXECUTABLE}"
    "${PROJECT_SOURCE_DIR}/tests/smoke.py" --qemu "${XV_QEMU}"
    --kernel $<TARGET_FILE:kernel> --image "${PROJECT_BINARY_DIR}/fs.img"
    --allocator "${XV_PAGE_ALLOCATOR}" --ram-mib "${XV_RAM_MB}"
    --jobs --cpus ${cpus})
  set_tests_properties(jobs_${cpus}cpu PROPERTIES TIMEOUT 90 LABELS tty)
  add_test(NAME foreground_${cpus}cpu COMMAND "${Python3_EXECUTABLE}"
    "${PROJECT_SOURCE_DIR}/tests/smoke.py" --qemu "${XV_QEMU}"
    --kernel $<TARGET_FILE:kernel> --image "${PROJECT_BINARY_DIR}/fs.img"
    --allocator "${XV_PAGE_ALLOCATOR}" --ram-mib "${XV_RAM_MB}"
    --foreground --cpus ${cpus})
  set_tests_properties(foreground_${cpus}cpu PROPERTIES TIMEOUT 90 LABELS tty)
  add_test(NAME signals_${cpus}cpu COMMAND "${Python3_EXECUTABLE}"
    "${PROJECT_SOURCE_DIR}/tests/smoke.py" --qemu "${XV_QEMU}"
    --kernel $<TARGET_FILE:kernel> --image "${PROJECT_BINARY_DIR}/fs.img"
    --allocator "${XV_PAGE_ALLOCATOR}" --ram-mib "${XV_RAM_MB}"
    --signals --cpus ${cpus})
  set_tests_properties(signals_${cpus}cpu PROPERTIES TIMEOUT 90 LABELS process)
  add_test(NAME process_groups_${cpus}cpu COMMAND "${Python3_EXECUTABLE}"
    "${PROJECT_SOURCE_DIR}/tests/smoke.py" --qemu "${XV_QEMU}"
    --kernel $<TARGET_FILE:kernel> --image "${PROJECT_BINARY_DIR}/fs.img"
    --allocator "${XV_PAGE_ALLOCATOR}" --ram-mib "${XV_RAM_MB}"
    --process-groups --cpus ${cpus})
  set_tests_properties(process_groups_${cpus}cpu PROPERTIES TIMEOUT 90 LABELS process)
  add_test(NAME tty_${cpus}cpu COMMAND "${Python3_EXECUTABLE}"
    "${PROJECT_SOURCE_DIR}/tests/smoke.py" --qemu "${XV_QEMU}"
    --kernel $<TARGET_FILE:kernel> --image "${PROJECT_BINARY_DIR}/fs.img"
    --allocator "${XV_PAGE_ALLOCATOR}" --ram-mib "${XV_RAM_MB}"
    --tty --boot-only --cpus ${cpus})
  set_tests_properties(tty_${cpus}cpu PROPERTIES TIMEOUT 90 LABELS tty)
  add_test(NAME process_scaling_${cpus}cpu COMMAND "${Python3_EXECUTABLE}"
    "${PROJECT_SOURCE_DIR}/tests/smoke.py" --qemu "${XV_QEMU}"
    --kernel $<TARGET_FILE:kernel> --image "${PROJECT_BINARY_DIR}/fs.img"
    --allocator "${XV_PAGE_ALLOCATOR}" --ram-mib "${XV_RAM_MB}"
    --process-scaling --cpus ${cpus})
  set_tests_properties(process_scaling_${cpus}cpu PROPERTIES TIMEOUT 90 LABELS process)
endforeach()

foreach(cpus 1 3)
  add_test(NAME network_${cpus}cpu COMMAND "${Python3_EXECUTABLE}"
    "${PROJECT_SOURCE_DIR}/tests/network.py" --qemu "${XV_QEMU}"
    --kernel $<TARGET_FILE:kernel> --image "${PROJECT_BINARY_DIR}/fs.img" --cpus ${cpus})
  set_tests_properties(network_${cpus}cpu PROPERTIES TIMEOUT 65 LABELS network)
endforeach()

add_test(NAME network_contract COMMAND "${PROJECT_BINARY_DIR}/network_contract")
set_tests_properties(network_contract PROPERTIES TIMEOUT 10 LABELS network)

foreach(cpus 1 3)
  add_test(NAME socket_${cpus}cpu COMMAND "${Python3_EXECUTABLE}"
    "${PROJECT_SOURCE_DIR}/tests/socket_network.py" --qemu "${XV_QEMU}"
    --kernel $<TARGET_FILE:kernel> --image "${PROJECT_BINARY_DIR}/fs.img" --cpus ${cpus})
  set_tests_properties(socket_${cpus}cpu PROPERTIES TIMEOUT 65 LABELS network)
endforeach()

foreach(cpus 1 3)
  add_test(NAME multinet_${cpus}cpu COMMAND "${Python3_EXECUTABLE}"
    "${PROJECT_SOURCE_DIR}/tests/network.py" --qemu "${XV_QEMU}"
    --kernel $<TARGET_FILE:kernel> --image "${PROJECT_BINARY_DIR}/fs.img" --cpus ${cpus} --multi)
  set_tests_properties(multinet_${cpus}cpu PROPERTIES TIMEOUT 65 LABELS network)
endforeach()

foreach(cpus 1 3)
  add_test(NAME netctl_${cpus}cpu COMMAND "${Python3_EXECUTABLE}"
    "${PROJECT_SOURCE_DIR}/tests/network.py" --qemu "${XV_QEMU}"
    --kernel $<TARGET_FILE:kernel> --image "${PROJECT_BINARY_DIR}/fs.img" --cpus ${cpus} --management)
  set_tests_properties(netctl_${cpus}cpu PROPERTIES TIMEOUT 65 LABELS network)
endforeach()

foreach(cpus 1 3)
  add_test(NAME bridge_${cpus}cpu COMMAND "${Python3_EXECUTABLE}"
    "${PROJECT_SOURCE_DIR}/tests/bridge_network.py" --qemu "${XV_QEMU}"
    --kernel $<TARGET_FILE:kernel> --image "${PROJECT_BINARY_DIR}/fs.img" --cpus ${cpus})
  set_tests_properties(bridge_${cpus}cpu PROPERTIES TIMEOUT 90 LABELS network)
endforeach()

foreach(cpus 1 3)
  add_test(NAME dhcp_${cpus}cpu COMMAND "${Python3_EXECUTABLE}"
    "${PROJECT_SOURCE_DIR}/tests/network.py" --qemu "${XV_QEMU}"
    --kernel $<TARGET_FILE:kernel> --image "${PROJECT_BINARY_DIR}/fs.img" --cpus ${cpus} --dhcp)
  set_tests_properties(dhcp_${cpus}cpu PROPERTIES TIMEOUT 90 LABELS network)
endforeach()

foreach(cpus 1 3)
  add_test(NAME dns_${cpus}cpu COMMAND "${Python3_EXECUTABLE}"
    "${PROJECT_SOURCE_DIR}/tests/dns_network.py" --qemu "${XV_QEMU}"
    --kernel $<TARGET_FILE:kernel> --image "${PROJECT_BINARY_DIR}/fs.img" --cpus ${cpus})
  set_tests_properties(dns_${cpus}cpu PROPERTIES TIMEOUT 90 LABELS network)
endforeach()

add_test(NAME dns_contract COMMAND "${PROJECT_BINARY_DIR}/dns_contract")
set_tests_properties(dns_contract PROPERTIES TIMEOUT 10 LABELS network)

foreach(cpus 1 3)
  add_test(NAME http_${cpus}cpu COMMAND "${Python3_EXECUTABLE}"
    "${PROJECT_SOURCE_DIR}/tests/http_network.py" --qemu "${XV_QEMU}"
    --kernel $<TARGET_FILE:kernel> --image "${PROJECT_BINARY_DIR}/fs.img" --cpus ${cpus})
  set_tests_properties(http_${cpus}cpu PROPERTIES TIMEOUT 120 LABELS network)
endforeach()

add_test(NAME http_contract COMMAND "${PROJECT_BINARY_DIR}/http_contract")
set_tests_properties(http_contract PROPERTIES TIMEOUT 10 LABELS network)
