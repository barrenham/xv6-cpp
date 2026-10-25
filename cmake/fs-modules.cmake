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
      "${module_root}/file/file_operations.cppm"
      "${module_root}/pipe/pipe.cppm"
      "${module_root}/platform/platform.cppm"
  )
  set_property(TARGET ${target} PROPERTY CXX_SCAN_FOR_MODULES ON)
endfunction()
