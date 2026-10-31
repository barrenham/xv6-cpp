# Build this tool in a separate native CMake project with its own clangd database.
include(ExternalProject)
ExternalProject_Add(host_mkfs
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/host"
  BINARY_DIR "${PROJECT_BINARY_DIR}/host"
  CMAKE_ARGS
    "-DCMAKE_CXX_COMPILER:FILEPATH=${XV_HOST_CLANG}"
    "-DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=ON"
    "-DXV_OUTPUT_DIR:PATH=${PROJECT_BINARY_DIR}"
    "-DXV_SLAB_ALLOCATOR:STRING=${XV_SLAB_ALLOCATOR}"
    "-DXV_DISPLAY_WIDTH:STRING=${XV_DISPLAY_WIDTH}"
    "-DXV_DISPLAY_HEIGHT:STRING=${XV_DISPLAY_HEIGHT}"
  BUILD_ALWAYS TRUE
  # Python's offline image adapter uses the same ext3 engine as the kernel.
  BUILD_BYPRODUCTS "${PROJECT_BINARY_DIR}/fbterm_contract" "${PROJECT_BINARY_DIR}/system_accounting_contract" "${PROJECT_BINARY_DIR}/exec_snapshot_contract" "${PROJECT_BINARY_DIR}/libext3_image.so" "${PROJECT_BINARY_DIR}/ext3_volume_contract" "${PROJECT_BINARY_DIR}/ext3_jbd_contract" "${PROJECT_BINARY_DIR}/slab_contract" "${PROJECT_BINARY_DIR}/support_containers_contract" "${PROJECT_BINARY_DIR}/scheduling_policy_contract" "${PROJECT_BINARY_DIR}/rb_tree_contract" "${PROJECT_BINARY_DIR}/flow_contract" "${PROJECT_BINARY_DIR}/vm_contract" "${PROJECT_BINARY_DIR}/password_hash_contract" "${PROJECT_BINARY_DIR}/http_contract" "${PROJECT_BINARY_DIR}/dns_contract" "${PROJECT_BINARY_DIR}/network_contract" "${PROJECT_BINARY_DIR}/process_groups_contract" "${PROJECT_BINARY_DIR}/process_storage_contract" "${PROJECT_BINARY_DIR}/runtime_contract" "${PROJECT_BINARY_DIR}/mkfs" "${PROJECT_BINARY_DIR}/allocator_contract" "${PROJECT_BINARY_DIR}/sync_contract" "${PROJECT_BINARY_DIR}/driver_contract" "${PROJECT_BINARY_DIR}/process_contract" "${PROJECT_BINARY_DIR}/fs_contract" "${PROJECT_BINARY_DIR}/trap_contract"
  INSTALL_COMMAND ""
)
