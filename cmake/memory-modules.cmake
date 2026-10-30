function(xv_add_memory_modules target)
  get_target_property(has_memory_modules ${target} XV_MEMORY_MODULES)
  if(has_memory_modules)
    return()
  endif()
  set_property(TARGET ${target} PROPERTY XV_MEMORY_MODULES TRUE)
  xv_add_virtual_pool_modules(${target})
  set(module_root "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../kernel/memory")
  target_sources(${target} PRIVATE
    FILE_SET memory_modules TYPE CXX_MODULES
    BASE_DIRS "${module_root}"
    FILES
      "${module_root}/paging/sv39_page_table.cppm"
      "${module_root}/paging/kernel_page_table.cppm"
      "${module_root}/paging/user_paging.cppm"
      "${module_root}/memory.cppm"
      "${module_root}/address/physical_address.cppm"
      "${module_root}/address/physical_memory_range.cppm"
      "${module_root}/layout/physical_memory_layout.cppm"
      "${module_root}/boot/boot_memory.cppm"
      "${module_root}/allocator/page_allocator.cppm"
      "${module_root}/slab/slab_cache.cppm"
      "${module_root}/allocator/allocator_detail.cppm"
      "${module_root}/allocator/buddy/buddy_page_allocator.cppm"
      "${module_root}/allocator/freelist/freelist_page_allocator.cppm"
  )
  set_property(TARGET ${target} PROPERTY CXX_SCAN_FOR_MODULES ON)
endfunction()

function(xv_add_virtual_pool_modules target)
  get_target_property(has_region_module ${target} XV_REGION_MODULE)
  if(has_region_module)
    return()
  endif()
  set_property(TARGET ${target} PROPERTY XV_REGION_MODULE TRUE)
  set(module_root "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../kernel/memory")
  target_sources(${target} PRIVATE FILE_SET region_module TYPE CXX_MODULES
    BASE_DIRS "${module_root}" FILES "${module_root}/layout/virtual_memory_types.cppm"
          "${module_root}/layout/virtual_address_pool.cppm")
endfunction()
