cmake_minimum_required(VERSION 3.28)
project(xv_labs_cpp_2026 VERSION 0.0.2 LANGUAGES CXX ASM)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(XV_RAM_MB 128 CACHE STRING "QEMU RAM size in MiB (kernel discovers it from DTB)")
if(NOT XV_RAM_MB MATCHES "^[1-9][0-9]*$")
  message(FATAL_ERROR "XV_RAM_MB must be a positive integer")
endif()
set(XV_KERNEL_STACK_MIB 2 CACHE STRING "Reserved virtual kernel stack size per process in MiB")
if(NOT XV_KERNEL_STACK_MIB MATCHES "^(2|4|8|16|32|64)$")
  message(FATAL_ERROR "XV_KERNEL_STACK_MIB must be a power of two between 2 and 64")
endif()
set(XV_CPUS 5 CACHE STRING "Number of QEMU harts")
set(XV_GDB_PORT 26000 CACHE STRING "QEMU GDB port")
find_program(XV_LLD ld.lld HINTS "$ENV{CONDA_PREFIX}/bin" REQUIRED)
find_program(XV_HOST_CLANG clang++ HINTS "$ENV{CONDA_PREFIX}/bin" REQUIRED)
find_program(XV_QEMU qemu-system-riscv64 HINTS "$ENV{CONDA_PREFIX}/bin" REQUIRED)
find_package(Python3 REQUIRED COMPONENTS Interpreter)
add_library(xv_options INTERFACE)
target_include_directories(xv_options INTERFACE "${PROJECT_SOURCE_DIR}")
target_compile_definitions(xv_options INTERFACE LAB_PGTBL SOL_PGTBL LAB_NET XV_KERNEL_STACK_MIB=${XV_KERNEL_STACK_MIB})
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
set(XV_FS_DIRECTORIES "")
foreach(directory /bin /sbin /usr /usr/tests /usr/share /usr/share/doc /usr/share/doc/xv6 /dev /proc /etc /home /data)
  string(APPEND XV_FS_DIRECTORIES "dir\t0755\t${directory}\n")
endforeach()
string(APPEND XV_FS_DIRECTORIES "dir\t01777\t/tmp\ndir\t0700\t/root\n")
file(GENERATE OUTPUT "${PROJECT_BINARY_DIR}/fs-manifest.tsv"
  CONTENT "${XV_FS_DIRECTORIES}file\t0644\t/usr/share/doc/xv6/README\t${PROJECT_BINARY_DIR}/README\n${XV_FS_ENTRIES}")
add_custom_command(OUTPUT "${PROJECT_BINARY_DIR}/fs.img"
  COMMAND "${PROJECT_BINARY_DIR}/mkfs" "${PROJECT_BINARY_DIR}/fs.img" --manifest "${PROJECT_BINARY_DIR}/fs-manifest.tsv"
  DEPENDS host_mkfs "${PROJECT_BINARY_DIR}/mkfs" "${PROJECT_BINARY_DIR}/fs-manifest.tsv" "${PROJECT_BINARY_DIR}/README" ${XV_USER_BINARIES}
  VERBATIM)
add_custom_target(image ALL DEPENDS "${PROJECT_BINARY_DIR}/fs.img")
# Keep test enrollment independent of the disk used by interactive QEMU sessions.
add_custom_command(OUTPUT "${PROJECT_BINARY_DIR}/fs-test.img"
  COMMAND "${PROJECT_BINARY_DIR}/mkfs" "${PROJECT_BINARY_DIR}/fs-test.img" --manifest "${PROJECT_BINARY_DIR}/fs-manifest.tsv"
  DEPENDS host_mkfs "${PROJECT_BINARY_DIR}/mkfs" "${PROJECT_BINARY_DIR}/fs-manifest.tsv" "${PROJECT_BINARY_DIR}/README" ${XV_USER_BINARIES}
  VERBATIM)
add_custom_target(test-image ALL DEPENDS "${PROJECT_BINARY_DIR}/fs-test.img")
set(XV_ROOTFS_IMAGE "${PROJECT_SOURCE_DIR}/data/rootfs.img" CACHE FILEPATH "Persistent root filesystem; initialized once from the build image")
set(XV_ROOTFS_IMPORT_IMAGE "" CACHE FILEPATH "Explicit optional legacy data volume; empty creates a clean rootfs")
set(XV_ROOTFS_IMPORT_OPTIONS "")
if(XV_ROOTFS_IMPORT_IMAGE)
  list(APPEND XV_ROOTFS_IMPORT_OPTIONS --import-data "${XV_ROOTFS_IMPORT_IMAGE}")
endif()
add_custom_target(rootfs-image
  COMMAND "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tools/create_rootfs.py"
    --seed "${PROJECT_BINARY_DIR}/fs.img" --output "${XV_ROOTFS_IMAGE}"
    ${XV_ROOTFS_IMPORT_OPTIONS}
  DEPENDS image VERBATIM)
add_custom_target(rootfs-create
  COMMAND "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tools/create_rootfs.py"
    --seed "${PROJECT_BINARY_DIR}/fs.img" --output "${XV_ROOTFS_IMAGE}" --fresh
  DEPENDS image VERBATIM)
add_custom_target(rootfs-update
  COMMAND "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tools/update_rootfs.py"
    --seed "${PROJECT_BINARY_DIR}/fs.img" --image "${XV_ROOTFS_IMAGE}"
    --manifest "${PROJECT_BINARY_DIR}/fs-manifest.tsv"
  DEPENDS image VERBATIM)
set(XV_QEMU_OPTIONS -machine virt -bios none -kernel $<TARGET_FILE:kernel> -m ${XV_RAM_MB}M -smp ${XV_CPUS} -nographic -global virtio-mmio.force-legacy=false -drive file=${XV_ROOTFS_IMAGE},if=none,format=raw,id=x0 -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0)
set(XV_NET_BACKEND "user,id=net0" CACHE STRING "QEMU network backend; append hostfwd for inbound UDP")
list(APPEND XV_QEMU_OPTIONS -netdev ${XV_NET_BACKEND} -device e1000,netdev=net0,bus=pcie.0,addr=01.0,mac=52:54:00:12:34:56)
add_custom_target(qemu COMMAND "${XV_QEMU}" ${XV_QEMU_OPTIONS} DEPENDS kernel rootfs-image USES_TERMINAL)
add_custom_target(qemu-gdb COMMAND "${XV_QEMU}" ${XV_QEMU_OPTIONS} -S -gdb tcp::${XV_GDB_PORT} DEPENDS kernel rootfs-image USES_TERMINAL)
enable_testing()
foreach(cpus IN ITEMS 1 5)
  add_test(NAME fork_progress_${cpus}cpu COMMAND "${Python3_EXECUTABLE}"
    "${PROJECT_SOURCE_DIR}/tests/fork_progress.py" --qemu "${XV_QEMU}"
    --kernel $<TARGET_FILE:kernel> --image "${PROJECT_BINARY_DIR}/fs-test.img"
    --cpus ${cpus} --output "${PROJECT_BINARY_DIR}/fork-progress-${cpus}cpu.json")
  set_tests_properties(fork_progress_${cpus}cpu PROPERTIES TIMEOUT 120 LABELS process)
endforeach()
add_test(NAME password_hash_contract COMMAND "${PROJECT_BINARY_DIR}/password_hash_contract")
set_tests_properties(password_hash_contract PROPERTIES TIMEOUT 15 LABELS users)
add_test(NAME image_layout COMMAND "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tests/image_layout.py"
  --mkfs "${PROJECT_BINARY_DIR}/mkfs" --image "${PROJECT_BINARY_DIR}/fs-test.img"
  --manifest "${PROJECT_BINARY_DIR}/fs-manifest.tsv")
set_tests_properties(image_layout PROPERTIES TIMEOUT 30 LABELS fs)
add_test(NAME signal_contract COMMAND "${PROJECT_BINARY_DIR}/process_groups_contract" --signals)
set_tests_properties(signal_contract PROPERTIES TIMEOUT 45 LABELS process)
add_test(NAME process_groups_contract COMMAND "${PROJECT_BINARY_DIR}/process_groups_contract")
set_tests_properties(process_groups_contract PROPERTIES TIMEOUT 45 LABELS process)
add_test(NAME process_storage_contract COMMAND "${PROJECT_BINARY_DIR}/process_storage_contract")
set_tests_properties(process_storage_contract PROPERTIES TIMEOUT 45 LABELS process)
add_test(NAME runtime_contract COMMAND "${PROJECT_BINARY_DIR}/runtime_contract")
add_test(NAME print_contract COMMAND "${PROJECT_BINARY_DIR}/print_contract")
set_tests_properties(print_contract PROPERTIES TIMEOUT 10 LABELS runtime)
set_tests_properties(runtime_contract PROPERTIES TIMEOUT 10 LABELS runtime)
add_test(NAME trap_contract COMMAND "${PROJECT_BINARY_DIR}/trap_contract")
set_tests_properties(trap_contract PROPERTIES TIMEOUT 45 LABELS trap)
add_test(NAME fs_contract COMMAND "${PROJECT_BINARY_DIR}/fs_contract" "${PROJECT_BINARY_DIR}/fs-test.img")
set_tests_properties(fs_contract PROPERTIES TIMEOUT 45 LABELS fs)
add_test(NAME process_contract COMMAND "${PROJECT_BINARY_DIR}/process_contract")
set_tests_properties(process_contract PROPERTIES TIMEOUT 45 LABELS process)
add_test(NAME driver_contract COMMAND "${PROJECT_BINARY_DIR}/driver_contract")
set_tests_properties(driver_contract PROPERTIES TIMEOUT 45 LABELS drivers)
add_test(NAME sync_contract COMMAND "${PROJECT_BINARY_DIR}/sync_contract")
set_tests_properties(sync_contract PROPERTIES TIMEOUT 45 LABELS sync)
add_test(NAME boot_smoke COMMAND "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tests/smoke.py" --qemu "${XV_QEMU}" --kernel $<TARGET_FILE:kernel> --image "${PROJECT_BINARY_DIR}/fs-test.img" --allocator "${XV_PAGE_ALLOCATOR}" --ram-mib "${XV_RAM_MB}")
set_tests_properties(boot_smoke PROPERTIES TIMEOUT 120)
add_test(NAME user_regression COMMAND "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tests/smoke.py" --qemu "${XV_QEMU}" --kernel $<TARGET_FILE:kernel> --image "${PROJECT_BINARY_DIR}/fs-test.img" --allocator "${XV_PAGE_ALLOCATOR}" --ram-mib "${XV_RAM_MB}" --regression)
set_tests_properties(user_regression PROPERTIES TIMEOUT 330)
foreach(kind buddy freelist)
  add_test(NAME allocator_contract_${kind} COMMAND "${PROJECT_BINARY_DIR}/allocator_contract" ${kind})
  set_tests_properties(allocator_contract_${kind} PROPERTIES TIMEOUT 45 LABELS memory)
endforeach()

# Same kernel binary, different firmware-described RAM lengths and mapping edges.
foreach(ram 64 257 2048 2049)
  add_test(NAME ram_${ram}MiB COMMAND "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tests/smoke.py"
    --qemu "${XV_QEMU}" --kernel $<TARGET_FILE:kernel> --image "${PROJECT_BINARY_DIR}/fs-test.img"
    --allocator "${XV_PAGE_ALLOCATOR}" --ram-mib ${ram} --boot-only)
  set_tests_properties(ram_${ram}MiB PROPERTIES TIMEOUT 120 LABELS ram)
endforeach()

foreach(cpus 1 5)
  foreach(feature named-pty procfs ramfs environment permissions workflow session)
    add_test(NAME ${feature}_${cpus}cpu COMMAND "${Python3_EXECUTABLE}"
      "${PROJECT_SOURCE_DIR}/tests/smoke.py" --qemu "${XV_QEMU}"
      --kernel $<TARGET_FILE:kernel> --image "${PROJECT_BINARY_DIR}/fs-test.img"
      --allocator "${XV_PAGE_ALLOCATOR}" --ram-mib "${XV_RAM_MB}"
      --${feature} --cpus ${cpus})
    set_tests_properties(${feature}_${cpus}cpu PROPERTIES TIMEOUT 120 LABELS fs)
  endforeach()

  add_test(NAME devfs_${cpus}cpu COMMAND "${Python3_EXECUTABLE}"
    "${PROJECT_SOURCE_DIR}/tests/smoke.py" --qemu "${XV_QEMU}"
    --kernel $<TARGET_FILE:kernel> --image "${PROJECT_BINARY_DIR}/fs-test.img"
    --allocator "${XV_PAGE_ALLOCATOR}" --ram-mib "${XV_RAM_MB}"
    --devfs --cpus ${cpus})
  set_tests_properties(devfs_${cpus}cpu PROPERTIES TIMEOUT 40 LABELS fs)
  add_test(NAME sigchld_${cpus}cpu COMMAND "${Python3_EXECUTABLE}"
    "${PROJECT_SOURCE_DIR}/tests/smoke.py" --qemu "${XV_QEMU}"
    --kernel $<TARGET_FILE:kernel> --image "${PROJECT_BINARY_DIR}/fs-test.img"
    --allocator "${XV_PAGE_ALLOCATOR}" --ram-mib "${XV_RAM_MB}"
    --sigchld --cpus ${cpus})
  set_tests_properties(sigchld_${cpus}cpu PROPERTIES TIMEOUT 30 LABELS process)
  add_test(NAME nonblock_${cpus}cpu COMMAND "${Python3_EXECUTABLE}"
    "${PROJECT_SOURCE_DIR}/tests/smoke.py" --qemu "${XV_QEMU}"
    --kernel $<TARGET_FILE:kernel> --image "${PROJECT_BINARY_DIR}/fs-test.img"
    --allocator "${XV_PAGE_ALLOCATOR}" --ram-mib "${XV_RAM_MB}"
    --nonblock --cpus ${cpus})
  set_tests_properties(nonblock_${cpus}cpu PROPERTIES TIMEOUT 20 LABELS io)
  add_test(NAME poll_${cpus}cpu COMMAND "${Python3_EXECUTABLE}"
    "${PROJECT_SOURCE_DIR}/tests/smoke.py" --qemu "${XV_QEMU}"
    --kernel $<TARGET_FILE:kernel> --image "${PROJECT_BINARY_DIR}/fs-test.img"
    --allocator "${XV_PAGE_ALLOCATOR}" --ram-mib "${XV_RAM_MB}"
    --poll --cpus ${cpus})
  set_tests_properties(poll_${cpus}cpu PROPERTIES TIMEOUT 15 LABELS io)
  add_test(NAME pty_${cpus}cpu COMMAND "${Python3_EXECUTABLE}"
    "${PROJECT_SOURCE_DIR}/tests/smoke.py" --qemu "${XV_QEMU}"
    --kernel $<TARGET_FILE:kernel> --image "${PROJECT_BINARY_DIR}/fs-test.img"
    --allocator "${XV_PAGE_ALLOCATOR}" --ram-mib "${XV_RAM_MB}"
    --pty --cpus ${cpus})
  set_tests_properties(pty_${cpus}cpu PROPERTIES TIMEOUT 35 LABELS tty)
  add_test(NAME jobs_${cpus}cpu COMMAND "${Python3_EXECUTABLE}"
    "${PROJECT_SOURCE_DIR}/tests/smoke.py" --qemu "${XV_QEMU}"
    --kernel $<TARGET_FILE:kernel> --image "${PROJECT_BINARY_DIR}/fs-test.img"
    --allocator "${XV_PAGE_ALLOCATOR}" --ram-mib "${XV_RAM_MB}"
    --jobs --cpus ${cpus})
  set_tests_properties(jobs_${cpus}cpu PROPERTIES TIMEOUT 90 LABELS tty)
  add_test(NAME foreground_${cpus}cpu COMMAND "${Python3_EXECUTABLE}"
    "${PROJECT_SOURCE_DIR}/tests/smoke.py" --qemu "${XV_QEMU}"
    --kernel $<TARGET_FILE:kernel> --image "${PROJECT_BINARY_DIR}/fs-test.img"
    --allocator "${XV_PAGE_ALLOCATOR}" --ram-mib "${XV_RAM_MB}"
    --foreground --cpus ${cpus})
  set_tests_properties(foreground_${cpus}cpu PROPERTIES TIMEOUT 90 LABELS tty)
  add_test(NAME signals_${cpus}cpu COMMAND "${Python3_EXECUTABLE}"
    "${PROJECT_SOURCE_DIR}/tests/smoke.py" --qemu "${XV_QEMU}"
    --kernel $<TARGET_FILE:kernel> --image "${PROJECT_BINARY_DIR}/fs-test.img"
    --allocator "${XV_PAGE_ALLOCATOR}" --ram-mib "${XV_RAM_MB}"
    --signals --cpus ${cpus})
  set_tests_properties(signals_${cpus}cpu PROPERTIES TIMEOUT 90 LABELS process)
  add_test(NAME process_groups_${cpus}cpu COMMAND "${Python3_EXECUTABLE}"
    "${PROJECT_SOURCE_DIR}/tests/smoke.py" --qemu "${XV_QEMU}"
    --kernel $<TARGET_FILE:kernel> --image "${PROJECT_BINARY_DIR}/fs-test.img"
    --allocator "${XV_PAGE_ALLOCATOR}" --ram-mib "${XV_RAM_MB}"
    --process-groups --cpus ${cpus})
  set_tests_properties(process_groups_${cpus}cpu PROPERTIES TIMEOUT 90 LABELS process)
  add_test(NAME tty_${cpus}cpu COMMAND "${Python3_EXECUTABLE}"
    "${PROJECT_SOURCE_DIR}/tests/smoke.py" --qemu "${XV_QEMU}"
    --kernel $<TARGET_FILE:kernel> --image "${PROJECT_BINARY_DIR}/fs-test.img"
    --allocator "${XV_PAGE_ALLOCATOR}" --ram-mib "${XV_RAM_MB}"
    --tty --boot-only --cpus ${cpus})
  set_tests_properties(tty_${cpus}cpu PROPERTIES TIMEOUT 90 LABELS tty)
  add_test(NAME process_scaling_${cpus}cpu COMMAND "${Python3_EXECUTABLE}"
    "${PROJECT_SOURCE_DIR}/tests/smoke.py" --qemu "${XV_QEMU}"
    --kernel $<TARGET_FILE:kernel> --image "${PROJECT_BINARY_DIR}/fs-test.img"
    --allocator "${XV_PAGE_ALLOCATOR}" --ram-mib "${XV_RAM_MB}"
    --process-scaling --cpus ${cpus})
  set_tests_properties(process_scaling_${cpus}cpu PROPERTIES TIMEOUT 90 LABELS process)
endforeach()

foreach(cpus 1 5)
  add_test(NAME network_${cpus}cpu COMMAND "${Python3_EXECUTABLE}"
    "${PROJECT_SOURCE_DIR}/tests/network.py" --qemu "${XV_QEMU}"
    --kernel $<TARGET_FILE:kernel> --image "${PROJECT_BINARY_DIR}/fs-test.img" --cpus ${cpus})
  set_tests_properties(network_${cpus}cpu PROPERTIES TIMEOUT 65 LABELS network)
endforeach()

add_test(NAME network_contract COMMAND "${PROJECT_BINARY_DIR}/network_contract")
set_tests_properties(network_contract PROPERTIES TIMEOUT 10 LABELS network)

foreach(cpus 1 5)
  add_test(NAME socket_${cpus}cpu COMMAND "${Python3_EXECUTABLE}"
    "${PROJECT_SOURCE_DIR}/tests/socket_network.py" --qemu "${XV_QEMU}"
    --kernel $<TARGET_FILE:kernel> --image "${PROJECT_BINARY_DIR}/fs-test.img" --cpus ${cpus})
  set_tests_properties(socket_${cpus}cpu PROPERTIES TIMEOUT 65 LABELS network)
endforeach()

foreach(cpus 1 5)
  add_test(NAME multinet_${cpus}cpu COMMAND "${Python3_EXECUTABLE}"
    "${PROJECT_SOURCE_DIR}/tests/network.py" --qemu "${XV_QEMU}"
    --kernel $<TARGET_FILE:kernel> --image "${PROJECT_BINARY_DIR}/fs-test.img" --cpus ${cpus} --multi)
  set_tests_properties(multinet_${cpus}cpu PROPERTIES TIMEOUT 65 LABELS network)
endforeach()

foreach(cpus 1 5)
  add_test(NAME netctl_${cpus}cpu COMMAND "${Python3_EXECUTABLE}"
    "${PROJECT_SOURCE_DIR}/tests/network.py" --qemu "${XV_QEMU}"
    --kernel $<TARGET_FILE:kernel> --image "${PROJECT_BINARY_DIR}/fs-test.img" --cpus ${cpus} --management)
  set_tests_properties(netctl_${cpus}cpu PROPERTIES TIMEOUT 65 LABELS network)
endforeach()

foreach(cpus 1 5)
  add_test(NAME bridge_${cpus}cpu COMMAND "${Python3_EXECUTABLE}"
    "${PROJECT_SOURCE_DIR}/tests/bridge_network.py" --qemu "${XV_QEMU}"
    --kernel $<TARGET_FILE:kernel> --image "${PROJECT_BINARY_DIR}/fs-test.img" --cpus ${cpus})
  set_tests_properties(bridge_${cpus}cpu PROPERTIES TIMEOUT 90 LABELS network)
endforeach()

foreach(cpus 1 5)
  add_test(NAME dhcp_${cpus}cpu COMMAND "${Python3_EXECUTABLE}"
    "${PROJECT_SOURCE_DIR}/tests/network.py" --qemu "${XV_QEMU}"
    --kernel $<TARGET_FILE:kernel> --image "${PROJECT_BINARY_DIR}/fs-test.img" --cpus ${cpus} --dhcp)
  set_tests_properties(dhcp_${cpus}cpu PROPERTIES TIMEOUT 90 LABELS network)
endforeach()

foreach(cpus 1 5)
  add_test(NAME dns_${cpus}cpu COMMAND "${Python3_EXECUTABLE}"
    "${PROJECT_SOURCE_DIR}/tests/dns_network.py" --qemu "${XV_QEMU}"
    --kernel $<TARGET_FILE:kernel> --image "${PROJECT_BINARY_DIR}/fs-test.img" --cpus ${cpus})
  set_tests_properties(dns_${cpus}cpu PROPERTIES TIMEOUT 90 LABELS network)
endforeach()

add_test(NAME dns_contract COMMAND "${PROJECT_BINARY_DIR}/dns_contract")
set_tests_properties(dns_contract PROPERTIES TIMEOUT 10 LABELS network)

foreach(cpus 1 5)
  add_test(NAME http_${cpus}cpu COMMAND "${Python3_EXECUTABLE}"
    "${PROJECT_SOURCE_DIR}/tests/http_network.py" --qemu "${XV_QEMU}"
    --kernel $<TARGET_FILE:kernel> --image "${PROJECT_BINARY_DIR}/fs-test.img" --cpus ${cpus})
  set_tests_properties(http_${cpus}cpu PROPERTIES TIMEOUT 120 LABELS network)
endforeach()

add_test(NAME http_contract COMMAND "${PROJECT_BINARY_DIR}/http_contract")
set_tests_properties(http_contract PROPERTIES TIMEOUT 10 LABELS network)

foreach(cpus 1 5)
  foreach(kind platform-progress persistence)
    set(extra "")
    if(kind STREQUAL "persistence")
      set(extra --persistent)
    endif()
    add_test(NAME ${kind}_${cpus}cpu COMMAND "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tests/platform_progress.py"
      --qemu "${XV_QEMU}" --kernel $<TARGET_FILE:kernel> --image "${PROJECT_BINARY_DIR}/fs-test.img"
      --mkfs "${PROJECT_BINARY_DIR}/mkfs" --cpus ${cpus} ${extra})
    set_tests_properties(${kind}_${cpus}cpu PROPERTIES TIMEOUT 180 LABELS fs)
  endforeach()
endforeach()

add_test(NAME rootfs_image_contract COMMAND "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tests/rootfs_image.py"
  --image "${PROJECT_BINARY_DIR}/fs-test.img" --qemu "${XV_QEMU}")
set_tests_properties(rootfs_image_contract PROPERTIES TIMEOUT 20 LABELS fs)

foreach(allocator buddy freelist)
  add_test(NAME vm_${allocator}_contract COMMAND "${PROJECT_BINARY_DIR}/vm_contract" ${allocator})
  set_tests_properties(vm_${allocator}_contract PROPERTIES TIMEOUT 30 LABELS memory)
endforeach()

# Only this implementation unit changes in diagnostic kernels; normal kernels
# contain no test hook. The regression harness builds private objects/ELFs.
set_source_files_properties("${PROJECT_SOURCE_DIR}/kernel/trap/arch/riscv/kernel_platform.cpp"
  TARGET_DIRECTORY kernel PROPERTIES COMPILE_DEFINITIONS XV_VM_SELF_TEST=0)
add_test(NAME vm_stack_contract COMMAND "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tests/vm_kernel.py"
  --qemu "${XV_QEMU}" --kernel $<TARGET_FILE:kernel> --image "${PROJECT_BINARY_DIR}/fs-test.img")
set_tests_properties(vm_stack_contract PROPERTIES TIMEOUT 90 LABELS memory)

add_test(NAME rootfs_update_contract COMMAND "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tests/rootfs_update.py"
  --image "${PROJECT_BINARY_DIR}/fs-test.img" --manifest "${PROJECT_BINARY_DIR}/fs-manifest.tsv" --qemu "${XV_QEMU}")
set_tests_properties(rootfs_update_contract PROPERTIES TIMEOUT 45 LABELS fs)
foreach(cpus 1 5)
  add_test(NAME rootfs_update_${cpus}cpu COMMAND "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tests/rootfs_update_guest.py"
    --qemu "${XV_QEMU}" --kernel $<TARGET_FILE:kernel> --image "${PROJECT_BINARY_DIR}/fs-test.img"
    --manifest "${PROJECT_BINARY_DIR}/fs-manifest.tsv" --cpus ${cpus})
  set_tests_properties(rootfs_update_${cpus}cpu PROPERTIES TIMEOUT 180 LABELS fs)
endforeach()

foreach(cpus 1 5)
  add_test(NAME firewall_${cpus}cpu COMMAND "${Python3_EXECUTABLE}"
    "${PROJECT_SOURCE_DIR}/tests/network.py" --qemu "${XV_QEMU}"
    --kernel $<TARGET_FILE:kernel> --image "${PROJECT_BINARY_DIR}/fs-test.img" --cpus ${cpus} --firewall)
  set_tests_properties(firewall_${cpus}cpu PROPERTIES TIMEOUT 90 LABELS network)
endforeach()

add_test(NAME flow_contract COMMAND "${PROJECT_BINARY_DIR}/flow_contract")
add_test(NAME flow_compile_checks COMMAND "${Python3_EXECUTABLE}"
  "${PROJECT_SOURCE_DIR}/tests/flow_compile_checks.py" --cxx "${XV_HOST_CLANG}")
set_tests_properties(flow_contract flow_compile_checks PROPERTIES TIMEOUT 60 LABELS support)

add_test(NAME stats_contract COMMAND "${PROJECT_BINARY_DIR}/stats_contract")
set_tests_properties(stats_contract PROPERTIES TIMEOUT 30 LABELS support)
