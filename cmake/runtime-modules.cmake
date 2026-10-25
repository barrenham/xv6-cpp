function(xv_add_runtime_module target)
  set(module_root "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../kernel/support")
  target_sources(${target} PRIVATE
    FILE_SET runtime_modules TYPE CXX_MODULES
    BASE_DIRS "${module_root}"
    FILES "${module_root}/runtime.cppm"
  )
  set_property(TARGET ${target} PROPERTY CXX_SCAN_FOR_MODULES ON)
endfunction()
