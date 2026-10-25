target_sources(kernel PRIVATE
  FILE_SET print_modules TYPE CXX_MODULES FILES print.cppm
)

include("${PROJECT_SOURCE_DIR}/cmake/runtime-modules.cmake")
xv_add_runtime_module(kernel)

target_sources(kernel PRIVATE
  compiler_memory.cpp
  cpp_runtime.cpp
  printk.cpp
  stats.cpp
  string.cpp
)
