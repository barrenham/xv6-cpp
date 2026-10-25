include("${PROJECT_SOURCE_DIR}/cmake/fs-modules.cmake")
xv_add_fs_interfaces(kernel)
xv_add_fs_internals(kernel)
target_sources(kernel PRIVATE
  cache/lru_block_cache.cpp
  journal/journal.cpp
  inode/xv6_filesystem.cpp
  path/path_operations.cpp
  file/open_file.cpp
  file/file_operations.cpp
  pipe/pipe.cpp
  pipe/pipe_factory.cpp
  platform/kernel_platform.cpp
  platform/reference_bridge.cpp
  service.cpp
)
