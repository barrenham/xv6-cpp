# Build this tool in a separate native CMake project with its own clangd database.
include(ExternalProject)
ExternalProject_Add(host_mkfs
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/host"
  BINARY_DIR "${PROJECT_BINARY_DIR}/host"
  CMAKE_ARGS
    "-DCMAKE_CXX_COMPILER:FILEPATH=${XV_HOST_CLANG}"
    "-DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=ON"
    "-DXV_OUTPUT_DIR:PATH=${PROJECT_BINARY_DIR}"
  BUILD_ALWAYS TRUE
  BUILD_BYPRODUCTS "${PROJECT_BINARY_DIR}/http_contract" "${PROJECT_BINARY_DIR}/dns_contract" "${PROJECT_BINARY_DIR}/network_contract" "${PROJECT_BINARY_DIR}/process_groups_contract" "${PROJECT_BINARY_DIR}/process_storage_contract" "${PROJECT_BINARY_DIR}/runtime_contract" "${PROJECT_BINARY_DIR}/mkfs" "${PROJECT_BINARY_DIR}/allocator_contract" "${PROJECT_BINARY_DIR}/sync_contract" "${PROJECT_BINARY_DIR}/driver_contract" "${PROJECT_BINARY_DIR}/process_contract" "${PROJECT_BINARY_DIR}/fs_contract" "${PROJECT_BINARY_DIR}/trap_contract"
  INSTALL_COMMAND ""
)
