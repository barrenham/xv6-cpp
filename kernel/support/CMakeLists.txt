target_sources(kernel PRIVATE
  FILE_SET print_modules TYPE CXX_MODULES FILES print.cppm
)

target_sources(kernel PRIVATE
  cpp_runtime.cpp
  printk.cpp
  stats.cpp
  string.cpp
)
