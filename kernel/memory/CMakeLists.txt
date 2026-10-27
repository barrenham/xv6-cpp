include("${PROJECT_SOURCE_DIR}/cmake/memory-modules.cmake")
xv_add_memory_modules(kernel)

target_sources(kernel PRIVATE
  layout/virtual_regions.cpp
  paging/sv39_page_table.cpp
  boot/boot_memory.cpp
  allocator/allocator_service.cpp
  allocator/buddy/buddy_page_allocator.cpp
  allocator/freelist/freelist_page_allocator.cpp
  boot/kalloc.cpp
  paging/vm.cpp
  paging/kernel_page_table.cpp
)
set(XV_PAGE_ALLOCATOR "buddy" CACHE STRING "Boot physical page allocator: buddy or freelist")
set_property(CACHE XV_PAGE_ALLOCATOR PROPERTY STRINGS buddy freelist)
if(XV_PAGE_ALLOCATOR STREQUAL "freelist")
  target_compile_definitions(kernel PRIVATE XV_DEFAULT_ALLOCATOR_FREELIST=1)
elseif(XV_PAGE_ALLOCATOR STREQUAL "buddy")
  target_compile_definitions(kernel PRIVATE XV_DEFAULT_ALLOCATOR_FREELIST=0)
else()
  message(FATAL_ERROR "XV_PAGE_ALLOCATOR must be buddy or freelist")
endif()
