include("${PROJECT_SOURCE_DIR}/cmake/net-modules.cmake")
xv_add_net_modules(kernel)
target_sources(kernel PRIVATE platform/kernel_platform.cpp)
