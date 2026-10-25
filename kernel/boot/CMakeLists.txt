target_sources(kernel PRIVATE
  FILE_SET boot_modules TYPE CXX_MODULES
  FILES boot.cppm state.cppm
)

target_sources(kernel PRIVATE
  entry.S
  entry.cpp
  main.cpp
  start.cpp
)
