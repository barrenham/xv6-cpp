target_sources(kernel PRIVATE
  FILE_SET net_modules TYPE CXX_MODULES FILES net.cppm platform/platform.cppm
)
target_sources(kernel PRIVATE stack.cpp platform/kernel_platform.cpp)
