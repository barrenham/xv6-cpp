include("${PROJECT_SOURCE_DIR}/cmake/process-modules.cmake")
xv_add_process_interfaces(kernel)
xv_add_process_internals(kernel)

target_sources(kernel PRIVATE
  lifecycle/process.cpp
  lifecycle/process_table.cpp
  lifecycle/service.cpp
  resources/process_resources.cpp
  memory/address_space.cpp
  scheduler/round_robin.cpp
  exec/elf_loader.cpp
  arch/swtch.S
)
