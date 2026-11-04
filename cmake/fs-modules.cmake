function(xv_add_fs_interfaces target)
  set(module_root "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../kernel/fs")
  target_sources(${target} PRIVATE
    FILE_SET fs_interfaces TYPE CXX_MODULES
    BASE_DIRS "${module_root}"
    FILES
      "${module_root}/fs.cppm"
      "${module_root}/cache/block_cache.cppm"
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
  set(all_modules
      cache/lru_block_cache.cppm userfs/userfs.cppm file/file_operations.cppm
      buffer/flow.cppm file/flow.cppm file/inode_backend_flow.cppm cache/flow.cppm
      vfs/path_flow.cppm vfs/mount_flow.cppm
      pipe/pipe.cppm pipe/flow.cppm platform/platform.cppm)
  set(module_files)
  foreach(module IN LISTS all_modules)
    list(APPEND module_files "${module_root}/${module}")
  endforeach()
  target_sources(${target} PRIVATE
    FILE_SET fs_internals TYPE CXX_MODULES
    BASE_DIRS "${module_root}"
    FILES
      ${module_files}
  )
  set_property(TARGET ${target} PROPERTY CXX_SCAN_FOR_MODULES ON)
endfunction()
