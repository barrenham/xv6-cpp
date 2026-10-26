target_sources(kernel PRIVATE
  FILE_SET syscall_modules TYPE CXX_MODULES
  FILES syscall.cppm arguments.cppm handlers.cppm
)

target_sources(kernel PRIVATE
  syscall.cpp
  arguments.cpp
  syspoll.cpp
  sysfile.cpp
  sysproc.cpp
  sysnet.cpp
  syssocket.cpp
  sysvm.cpp
)
