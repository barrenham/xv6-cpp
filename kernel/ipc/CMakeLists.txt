target_sources(kernel PRIVATE
  FILE_SET ipc_modules TYPE CXX_MODULES FILES ipc.cppm
)
target_sources(kernel PRIVATE local.cpp shared.cpp)
