cmake_minimum_required(VERSION 3.28)
project(xv_mkfs_host LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
# Keep output paths valid when CMake resets a native compiler cache during upgrades.
get_filename_component(default_output_dir "${CMAKE_BINARY_DIR}/.." ABSOLUTE)
set(XV_OUTPUT_DIR "${default_output_dir}" CACHE PATH "Native tool output directory")
add_executable(mkfs ../main.cpp)
target_compile_features(mkfs PRIVATE cxx_std_20)
target_compile_options(mkfs PRIVATE -O2 -Wall -Wextra -Werror)
target_include_directories(mkfs PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../..")
set_target_properties(mkfs PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${XV_OUTPUT_DIR}")

# Native contract tests use the same allocator/service sources as the kernel.
find_package(Threads REQUIRED)
set(XV_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../..")
add_executable(allocator_contract
  "${XV_ROOT}/tests/allocator_contract.cpp"
  "${XV_ROOT}/tests/allocator_test_platform.cpp"
  "${XV_ROOT}/kernel/memory/paging/sv39_page_table.cpp"
  "${XV_ROOT}/kernel/memory/boot/boot_memory.cpp"
  "${XV_ROOT}/kernel/memory/allocator/allocator_service.cpp"
  "${XV_ROOT}/kernel/memory/allocator/buddy/buddy_page_allocator.cpp"
  "${XV_ROOT}/kernel/memory/allocator/freelist/freelist_page_allocator.cpp")
target_include_directories(allocator_contract PRIVATE "${XV_ROOT}")
target_compile_features(allocator_contract PRIVATE cxx_std_20)
target_compile_options(allocator_contract PRIVATE -O2 -Wall -Wextra -Werror -fno-rtti -fno-exceptions)
target_link_libraries(allocator_contract PRIVATE Threads::Threads)
set_target_properties(allocator_contract PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${XV_OUTPUT_DIR}")

include("${XV_ROOT}/cmake/memory-modules.cmake")
xv_add_memory_modules(allocator_contract)

include("${XV_ROOT}/cmake/sync-modules.cmake")
xv_add_sync_modules(allocator_contract)
target_sources(allocator_contract PRIVATE
  "${XV_ROOT}/kernel/sync/spin/spinlock.cpp"
  "${XV_ROOT}/kernel/sync/sleep/sleeplock.cpp"
  "${XV_ROOT}/tests/sync_test_platform.cpp")

add_executable(sync_contract
  "${XV_ROOT}/tests/sync_contract.cpp"
  "${XV_ROOT}/tests/sync_test_platform.cpp"
  "${XV_ROOT}/kernel/sync/spin/spinlock.cpp"
  "${XV_ROOT}/kernel/sync/sleep/sleeplock.cpp")
target_compile_features(sync_contract PRIVATE cxx_std_20)
target_compile_options(sync_contract PRIVATE -O2 -Wall -Wextra -Werror -fno-rtti -fno-exceptions)
target_link_libraries(sync_contract PRIVATE Threads::Threads)
set_target_properties(sync_contract PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${XV_OUTPUT_DIR}")
xv_add_sync_modules(sync_contract)
