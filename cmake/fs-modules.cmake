function(xv_add_fs_interfaces target)
  set(module_root "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../kernel/fs")
  target_sources(${target} PRIVATE
    FILE_SET fs_interfaces TYPE CXX_MODULES
    BASE_DIRS "${module_root}"
    FILES
      "${module_root}/fs.cppm"
      "${module_root}/cache/block_cache.cppm"
      "${module_root}/journal/journal.cppm"
      "${module_root}/inode/inode.cppm"
      "${module_root}/inode/filesystem.cppm"
      "${module_root}/buffer/page_buffer.cppm"
      "${module_root}/vfs/vfs.cppm"
      "${module_root}/file/open_file.cppm"
  )
  set_property(TARGET ${target} PROPERTY CXX_SCAN_FOR_MODULES ON)
endfunction()

function(xv_add_fs_internals target)
  set(module_root "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../kernel/fs")
  target_sources(${target} PRIVATE
    FILE_SET fs_internals TYPE CXX_MODULES
    BASE_DIRS "${module_root}"
    FILES
      "${module_root}/cache/lru_block_cache.cppm"
      "${module_root}/inode/xv6_filesystem.cppm"
      "${module_root}/devfs/devfs.cppm"
      "${module_root}/procfs/procfs.cppm"
      "${module_root}/ramfs/ramfs.cppm"
      "${module_root}/file/file_operations.cppm"
      "${module_root}/buffer/flow.cppm"
      "${module_root}/journal/flow.cppm"
      "${module_root}/file/flow.cppm"
      "${module_root}/file/inode_backend_flow.cppm"
      "${module_root}/cache/flow.cppm"
      "${module_root}/vfs/path_flow.cppm"
      "${module_root}/vfs/mount_flow.cppm"
      "${module_root}/vfs/initialize_flow.cppm"
      "${module_root}/ramfs/flow.cppm"
      "${module_root}/devfs/flow.cppm"
      "${module_root}/inode/flow.cppm"
      "${module_root}/path/flow.cppm"
      "${module_root}/pipe/pipe.cppm"
      "${module_root}/pipe/flow.cppm"
      "${module_root}/platform/platform.cppm"
  )
  set_property(TARGET ${target} PROPERTY CXX_SCAN_FOR_MODULES ON)
endfunction()
