include("${PROJECT_SOURCE_DIR}/cmake/sync-modules.cmake")
xv_add_sync_modules(kernel)

target_sources(kernel PRIVATE
  spin/spinlock.cpp
  sleep/sleeplock.cpp
  platform/kernel_platform.cpp
  platform/kernel_bridge.cpp
)
