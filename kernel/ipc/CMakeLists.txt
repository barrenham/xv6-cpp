target_sources(kernel PRIVATE
  FILE_SET ipc_modules TYPE CXX_MODULES FILES ipc.cppm
)
target_sources(kernel PRIVATE local.cpp named.cpp shared.cpp mqueue.cpp)
