function(xv_add_memory_modules target)
  set(module_root "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../kernel/memory")
  target_sources(${target} PRIVATE
    FILE_SET memory_modules TYPE CXX_MODULES
    BASE_DIRS "${module_root}"
    FILES
      "${module_root}/paging/sv39_page_table.cppm"
      "${module_root}/memory.cppm"
      "${module_root}/address/physical_address.cppm"
      "${module_root}/address/physical_memory_range.cppm"
      "${module_root}/layout/physical_memory_layout.cppm"
      "${module_root}/boot/boot_memory.cppm"
      "${module_root}/allocator/page_allocator.cppm"
      "${module_root}/allocator/allocator_detail.cppm"
      "${module_root}/allocator/buddy/buddy_page_allocator.cppm"
      "${module_root}/allocator/freelist/freelist_page_allocator.cppm"
  )
  set_property(TARGET ${target} PROPERTY CXX_SCAN_FOR_MODULES ON)
endfunction()
