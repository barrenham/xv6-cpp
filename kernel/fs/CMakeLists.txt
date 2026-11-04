include("${PROJECT_SOURCE_DIR}/cmake/fs-modules.cmake")
xv_add_fs_interfaces(kernel)
xv_add_fs_internals(kernel SERVICES_ONLY)
target_sources(kernel PRIVATE
  cache/lru_block_cache.cpp
  buffer/page_buffer.cpp
  file/device_registry.cpp
  file/service_resources.cpp
  file/bootstrap_block.cpp
  file/boot_asset.cpp
  file/open_file.cpp
  file/file_operations.cpp
  pipe/pipe.cpp
  pipe/pipe_factory.cpp
  platform/kernel_platform.cpp
  vfs/vfs.cpp
  userfs/userfs.cpp
  userfs/registry.cpp
  userfs/mount_manager.cpp
  permissions.cpp
  service.cpp
)
# Typed resource queries run on long-lived service stacks. Keep each handler's
# own frame bounded; copying complete tables here can permanently grow a stack.
set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/file/service_resources.cpp"
  TARGET_DIRECTORY kernel PROPERTIES COMPILE_OPTIONS "-Wframe-larger-than=1024")
