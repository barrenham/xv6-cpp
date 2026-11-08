cmake_minimum_required(VERSION 3.28)
project(independent_fs_driver LANGUAGES CXX ASM)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(XVFS_KIT_ROOT "" CACHE PATH "Directory containing the SDK/runtime/header kit")
if(NOT EXISTS "${XVFS_KIT_ROOT}/user/fs/sdk/server.cppm")
  message(FATAL_ERROR "Set XVFS_KIT_ROOT to the SDK source kit")
endif()
set(XVFS_DRIVER_NAME ext4 CACHE STRING "ELF name and filesystem type")
set(XVFS_DRIVER_INTERFACE "${XVFS_KIT_ROOT}/user/fs/backends/ext4/backend.cppm" CACHE FILEPATH "Driver module")
set(XVFS_DRIVER_IMPLEMENTATIONS "${XVFS_KIT_ROOT}/user/fs/backends/ext4/backend.cpp;${XVFS_KIT_ROOT}/user/fs/backends/ext4/volume.cpp" CACHE STRING "Driver source files")
set(XVFS_DRIVER_ENTRY "${XVFS_KIT_ROOT}/user/fs/plugins/ext4/main.cpp" CACHE FILEPATH "Driver main")
set(XVFS_DRIVER_RESOURCE block CACHE STRING "Source capability category")
set(XVFS_DRIVER_SOURCE 1 CACHE STRING "Default source")
add_compile_options(-O1 -Wall -Wextra -Werror -Wno-unused-parameter -Wno-sign-compare -Wno-write-strings -Wno-writable-strings)
include_directories("${XVFS_KIT_ROOT}" "${XVFS_KIT_ROOT}/user/include")
add_library(sdk STATIC
  "${XVFS_KIT_ROOT}/user/fs/sdk/server.cpp"
  "${XVFS_KIT_ROOT}/user/fs/sdk/thread_pool.cpp"
  "${XVFS_KIT_ROOT}/user/fs/sdk/shared_storage.cpp"
  "${XVFS_KIT_ROOT}/user/fs/sdk/dispatch.cpp"
  "${XVFS_KIT_ROOT}/user/fs/sdk/plugin.cpp"
  "${XVFS_KIT_ROOT}/user/fs/sdk/block_io.cpp")
target_sources(sdk PUBLIC FILE_SET sdk_modules TYPE CXX_MODULES
  BASE_DIRS "${XVFS_KIT_ROOT}/user/fs/sdk" FILES
  "${XVFS_KIT_ROOT}/user/fs/sdk/server.cppm"
  "${XVFS_KIT_ROOT}/user/fs/sdk/storage.cppm"
  "${XVFS_KIT_ROOT}/user/fs/sdk/shared_storage.cppm"
  "${XVFS_KIT_ROOT}/user/fs/sdk/plugin.cppm"
  "${XVFS_KIT_ROOT}/user/fs/sdk/block_io.cppm")
get_filename_component(driver_directory "${XVFS_DRIVER_INTERFACE}" DIRECTORY)
add_library(backend STATIC ${XVFS_DRIVER_IMPLEMENTATIONS})
target_sources(backend PUBLIC FILE_SET driver_module TYPE CXX_MODULES
  BASE_DIRS "${driver_directory}" FILES "${XVFS_DRIVER_INTERFACE}")
target_link_libraries(backend PUBLIC sdk)
set(runtime_sources "")
foreach(source ulib.cpp posix.cpp cxx.cpp path.cpp file_copy.cpp environment.cpp pty.cpp signals.cpp termios.cpp printf.cpp umalloc.cpp threads/tls.cpp threads/pthread.cpp threads/sync.cpp threads/timed.cpp threads/rwlock.cpp usys.S start.S process_entry.S)
  list(APPEND runtime_sources "${XVFS_KIT_ROOT}/user/runtime/${source}")
endforeach()
add_executable(driver "${XVFS_DRIVER_ENTRY}" ${runtime_sources})
target_link_libraries(driver PRIVATE backend)
target_link_options(driver PRIVATE -z max-page-size=4096 -e _user_start -T "${XVFS_KIT_ROOT}/linker/user.ld")
set_target_properties(driver PROPERTIES OUTPUT_NAME "${XVFS_DRIVER_NAME}" SUFFIX .elf
  LINK_DEPENDS "${XVFS_KIT_ROOT}/linker/user.ld")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${XVFS_DRIVER_NAME}.conf"
  "abi=1\ntype=${XVFS_DRIVER_NAME}\nexec=/usr/lib/fsplugins/${XVFS_DRIVER_NAME}.elf\nresource=${XVFS_DRIVER_RESOURCE}\nsource=${XVFS_DRIVER_SOURCE}\n")
