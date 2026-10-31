target_sources(kernel PRIVATE hardware.cpp)
target_sources(kernel PRIVATE
  FILE_SET arch_modules TYPE CXX_MODULES FILES arch.cppm
)
