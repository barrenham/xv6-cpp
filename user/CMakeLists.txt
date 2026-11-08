find_program(XV_USER_OBJCOPY llvm-objcopy HINTS "$ENV{CONDA_PREFIX}/bin" REQUIRED)
add_library(user_runtime OBJECT runtime/ulib.cpp runtime/posix.cpp runtime/cxx.cpp runtime/path.cpp runtime/file_copy.cpp runtime/environment.cpp runtime/pty.cpp runtime/signals.cpp runtime/termios.cpp runtime/printf.cpp runtime/umalloc.cpp runtime/threads/tls.cpp runtime/threads/pthread.cpp runtime/threads/sync.cpp runtime/threads/timed.cpp runtime/threads/rwlock.cpp runtime/usys.S runtime/start.S runtime/process_entry.S)
target_link_libraries(user_runtime PRIVATE xv_options)
add_library(user_services STATIC services/config.cpp services/control.cpp
  services/supervisor.cpp services/guardian.cpp services/log.cpp
  services/daemon.cpp services/bootstrap.cpp)
target_link_libraries(user_services PUBLIC xv_options)
add_subdirectory(fs)
add_library(user_accounts STATIC runtime/accounts.cpp runtime/account_admin.cpp runtime/authentication.cpp)
target_link_libraries(user_accounts PRIVATE xv_options)
add_library(user_dns STATIC runtime/dns/codec.cpp runtime/dns/resolver.cpp)
target_link_libraries(user_dns PRIVATE xv_options)
add_library(user_http STATIC runtime/http/codec.cpp runtime/http/client.cpp)
target_link_libraries(user_http PRIVATE xv_options PUBLIC user_dns)
function(xv_add_program source)
  get_filename_component(name "${source}" NAME_WE)
  if(ARGC GREATER 1)
    set(name "${ARGV1}")
  endif()
  set(target "user_${name}")
  if(name STREQUAL "fsboot")
    set(target "user_fsboot_executable")
  endif()
  add_executable(${target} "${source}" $<TARGET_OBJECTS:user_runtime>)
  if(source MATCHES "^apps/")
    get_filename_component(application_directory "${source}" DIRECTORY)
    include("${CMAKE_CURRENT_SOURCE_DIR}/${application_directory}/sources.cmake")
    get_filename_component(primary_source "${source}" NAME)
    foreach(application_source IN LISTS XV_APP_SOURCES)
      if(NOT application_source STREQUAL primary_source)
        target_sources(${target} PRIVATE "${application_directory}/${application_source}")
      endif()
    endforeach()
  endif()
  set_property(TARGET ${target} PROPERTY CXX_SCAN_FOR_MODULES ON)
  target_link_libraries(${target} PRIVATE xv_options user_http user_dns user_fs_core)
  target_link_options(${target} PRIVATE -z max-page-size=4096 -e _user_start -T "${PROJECT_SOURCE_DIR}/linker/user.ld")
  set_target_properties(${target} PROPERTIES OUTPUT_NAME _${name} RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/user" LINK_DEPENDS "${PROJECT_SOURCE_DIR}/linker/user.ld")
  set(guest_name "${name}")
  if(name STREQUAL "ramfsservicetest")
    set(guest_name "ramfssvctest")
  elseif(name STREQUAL "fsresourcesboottest")
    set(guest_name "fsresboot")
  elseif(name STREQUAL "demandcachetest")
    set(guest_name "demandcache")
  elseif(name STREQUAL "fsconcurrencytest")
    set(guest_name "fsconcurtest")
  elseif(name STREQUAL "fsparallelbench")
    set(guest_name "fsparbench")
  elseif(name STREQUAL "schedswitchtest")
    set(guest_name "schedswitch")
  endif()
  if(source MATCHES "^tests/" OR name STREQUAL "cppdemo" OR name STREQUAL "zombie")
    set(destination "/usr/tests/${guest_name}")
  elseif(name STREQUAL "serviced" OR name STREQUAL "fsboot" OR name STREQUAL "init" OR name STREQUAL "useradd" OR name STREQUAL "userdel" OR name STREQUAL "groupadd" OR name STREQUAL "groupdel" OR name STREQUAL "usermod" OR name STREQUAL "netctl" OR name STREQUAL "iptables")
    set(destination "/sbin/${name}")
  else()
    set(destination "/bin/${name}")
  endif()
  if(ARGC GREATER 2)
    set(destination "${ARGV2}")
  endif()
  # Internal tests share a manifest across ext3 and the retained xv6 format.
  # Keep source/target names descriptive, and use explicit installed aliases.
  if(source MATCHES "^tests/")
    get_filename_component(installed_name "${destination}" NAME)
    string(LENGTH "${installed_name}" installed_name_length)
    if(installed_name_length GREATER 14)
      message(FATAL_ERROR "Test installation name '${installed_name}' exceeds xv6's 14-byte limit; add an explicit guest_name alias")
    endif()
  endif()
  set(mode 0755)
  if(name STREQUAL "passwd")
    set(mode 04755)
  endif()
  # Guest loaders use program headers; keep full DWARF in the host executable
  # while installing a regular stripped ELF within the legacy xv6 file limit.
  set(guest_binary "${PROJECT_BINARY_DIR}/user/image/_${name}")
  if(name STREQUAL "fsboot")
    set(guest_binary "${PROJECT_BINARY_DIR}/user/image/_fsboot.base")
  endif()
  add_custom_command(TARGET ${target} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${PROJECT_BINARY_DIR}/user/image"
    COMMAND "${XV_USER_OBJCOPY}" --strip-debug $<TARGET_FILE:${target}> "${guest_binary}"
    BYPRODUCTS "${guest_binary}" VERBATIM)
  set(XV_FS_ENTRIES "${XV_FS_ENTRIES}file\t${mode}\t${destination}\t${guest_binary}\n" PARENT_SCOPE)
  set(XV_USER_BINARIES ${XV_USER_BINARIES} $<TARGET_FILE:${target}> "${guest_binary}" PARENT_SCOPE)
endfunction()
set(programs
  tests/threadio.cpp
  tests/threadstop.cpp
  tests/threadexec.cpp
  tests/pthreadsync.cpp
  tests/pthreadtest.cpp
  tests/threadcore.cpp
  tests/sharedheap.cpp
  tests/fspoolbench.cpp
  tests/fspooltest.cpp
  tests/cachereclaim.cpp
  tests/demandcachetest.cpp
  tests/fsworkertest.cpp
  tests/servicefixture.cpp
  tests/servicetest.cpp
  tests/filemaptest.cpp
  tests/mappingtest.cpp
  tests/unixsockettest.cpp
  tests/mqtest.cpp
  tests/posixipc.cpp
  tests/fscachetest.cpp
  tests/fsconcurrencytest.cpp
  tests/fsparallelbench.cpp
  tests/ipcbench.cpp
  tests/namedptytest.cpp
  tests/procfstest.cpp
  tests/ramfstest.cpp
  tests/ramfsservicetest.cpp
  tests/fsrootboottest.cpp
  tests/xv6fstest.cpp
  tests/ext4test.cpp
  tests/kmodtest.cpp
  tests/fsdrivertest.cpp
  tests/fsboottest.cpp
  tests/fsresourcesboottest.cpp
  tests/fsmgrboottest.cpp
  tests/fsmgrtest.cpp
  tests/fsdiskboot.cpp
  tests/fssnapshottest.cpp
  tests/userfstest.cpp
  tests/fsplugintest.cpp
  tests/cachebench.cpp
  tests/timerbench.cpp
  tests/schedulercheck.cpp
  tests/schedswitchtest.cpp
  tests/storagecheck.cpp
  tests/vmcheck.cpp
  tests/workflowtest.cpp
  tests/envtest.cpp
  tests/permissiontest.cpp
  tests/httptest.cpp
  tests/dnstest.cpp
  tests/devfstest.cpp
  tests/dhcptest.cpp
  tests/bridgetest.cpp
  tests/firewalltest.cpp
  tests/netctltest.cpp
  tests/nettest.cpp
  tests/multinet.cpp
  tests/sockettest.cpp
  tests/tcpduplex.cpp
  tests/ptytest.cpp
  tests/dorphan.cpp
  tests/forktest.cpp
  tests/forphan.cpp
  tests/grind.cpp
  tests/logstress.cpp
  tests/procext.cpp
  tests/grouptest.cpp
  tests/signaltest.cpp
  tests/fgtest.cpp
  tests/waiteventtest.cpp
  tests/jobtest.cpp
  tests/ttytest.cpp
  tests/pgtblext.cpp
  tests/pgtbltest.cpp
  tests/stressfs.cpp
  tests/superpgfork.cpp
  tests/superpgtest.cpp
  tests/usertests.cpp

)
include(apps/programs.cmake)
foreach(source IN LISTS programs)
  xv_add_program("${source}")
endforeach()
set(XV_USER_BINARIES ${XV_USER_BINARIES} PARENT_SCOPE)
string(APPEND XV_FS_ENTRIES "file\t0644\t/usr/share/doc/xv6/fbfont.txt\t${CMAKE_CURRENT_SOURCE_DIR}/apps/terminal/fbterm/FONT-LICENSE.txt\n")
set(XV_FS_ENTRIES "${XV_FS_ENTRIES}" PARENT_SCOPE)
set(plugin_sample "${PROJECT_BINARY_DIR}/fs-plugin-sample.tar")
add_custom_command(OUTPUT "${plugin_sample}"
  COMMAND "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tools/make_fs_plugin_sample.py" --output "${plugin_sample}"
  DEPENDS "${PROJECT_SOURCE_DIR}/tools/make_fs_plugin_sample.py" VERBATIM)
add_custom_target(fs-plugin-sample DEPENDS "${plugin_sample}")
string(APPEND XV_FS_ENTRIES "file\t0644\t/usr/share/doc/xv6/plugin.tar\t${plugin_sample}\n")
set(XV_FS_ENTRIES "${XV_FS_ENTRIES}" PARENT_SCOPE)
set(XV_FS_ASSETS "${CMAKE_CURRENT_SOURCE_DIR}/apps/terminal/fbterm/FONT-LICENSE.txt" "${plugin_sample}" PARENT_SCOPE)


# Poll contracts reuse the PTY test executable and installed inode.
target_sources(user_ptytest PRIVATE tests/poll_checks.cpp tests/nonblock_checks.cpp)

target_sources(user_signaltest PRIVATE tests/signal_handlers.cpp tests/child_signals.cpp tests/signal_registers.S)

foreach(program init id su useradd chown sh login passwd userdel groupadd groupdel usermod)
  target_link_libraries(user_${program} PRIVATE user_accounts)
endforeach()

target_link_libraries(user_ramfsd PRIVATE user_fs_ramfs)
target_link_libraries(user_sharedheap PRIVATE user_fs_shared)
target_link_libraries(user_fsrootboottest PRIVATE user_fs_block user_fs_ext3 user_fs_devfs)
target_link_libraries(user_fsdiskboot PRIVATE user_fs_block)
target_link_libraries(user_fsboot_executable PRIVATE user_fs_bootstrap)
target_link_libraries(user_fs_bootstrap PUBLIC user_services)
target_link_libraries(user_serviced PRIVATE user_services)
target_link_libraries(user_servicectl PRIVATE user_services)
target_link_libraries(user_servicetest PRIVATE user_services)
string(APPEND XV_FS_ENTRIES "file\t0644\t/etc/services.conf\t${CMAKE_CURRENT_SOURCE_DIR}/services/services.conf\n")
list(APPEND XV_FS_ASSETS "${CMAKE_CURRENT_SOURCE_DIR}/services/services.conf")

set(plugin_bundle_arguments --asset "boot.cfg=${CMAKE_CURRENT_SOURCE_DIR}/fs/plugins/boot.cfg")
set(plugin_bundle_dependencies "${CMAKE_CURRENT_SOURCE_DIR}/fs/plugins/boot.cfg")
list(APPEND plugin_bundle_arguments
  --asset "drivers/boot.list=${PROJECT_SOURCE_DIR}/drivers/boot.list"
  --asset "drivers/vblk.elf=${XV_KMOD_ELF}")
list(APPEND plugin_bundle_dependencies "${PROJECT_SOURCE_DIR}/drivers/boot.list" "${XV_KMOD_ELF}")
list(APPEND plugin_bundle_arguments --asset "drivers/rng.elf=${PROJECT_BINARY_DIR}/drivers/rng.elf")
list(APPEND plugin_bundle_dependencies "${PROJECT_BINARY_DIR}/drivers/rng.elf")
list(APPEND plugin_bundle_arguments --asset "drivers/mem.elf=${PROJECT_BINARY_DIR}/drivers/mem.elf")
list(APPEND plugin_bundle_dependencies "${PROJECT_BINARY_DIR}/drivers/mem.elf")
foreach(device_backend gpu input serial e1000)
  list(APPEND plugin_bundle_arguments
    --asset "drivers/${device_backend}.elf=${PROJECT_BINARY_DIR}/drivers/${device_backend}.elf")
  list(APPEND plugin_bundle_dependencies "${PROJECT_BINARY_DIR}/drivers/${device_backend}.elf")
endforeach()
foreach(backend ext3 xv6 devfs ramfs procfs)
  set(plugin_program "${backend}_plugin")
  xv_add_program("fs/plugins/${backend}/main.cpp" "${plugin_program}" "/usr/lib/fsplugins/${backend}.elf")
  target_link_libraries(user_${plugin_program} PRIVATE user_fs_${backend} user_fs_plugin)
  if(backend STREQUAL "ramfs")
    target_link_libraries(user_${plugin_program} PRIVATE user_fs_shared)
  endif()
  if(backend STREQUAL "ext3" OR backend STREQUAL "xv6")
    target_link_libraries(user_${plugin_program} PRIVATE user_fs_block)
  endif()
  set(config "${CMAKE_CURRENT_SOURCE_DIR}/fs/plugins/${backend}/${backend}.conf")
  string(APPEND XV_FS_ENTRIES "file\t0644\t/etc/fsplugins/${backend}.conf\t${config}\n")
  list(APPEND plugin_bundle_arguments --asset "fsplugins/${backend}.conf=${config}"
    --executable "fsplugins/${backend}.elf=${PROJECT_BINARY_DIR}/user/image/_${plugin_program}")
  list(APPEND plugin_bundle_dependencies "${config}" "${PROJECT_BINARY_DIR}/user/image/_${plugin_program}")
endforeach()
# Optional drivers are independent artifacts, never built into the boot catalog.
option(XV_INSTALL_EXT4_PLUGIN "Include the optional ext4 ELF in newly generated disk images" OFF)
set(core_fs_entries "${XV_FS_ENTRIES}")
set(core_user_binaries "${XV_USER_BINARIES}")
xv_add_program("fs/plugins/ext4/main.cpp" "ext4_plugin" "/usr/lib/fsplugins/ext4.elf")
target_link_libraries(user_ext4_plugin PRIVATE user_fs_ext4 user_fs_plugin user_fs_block)
if(XV_INSTALL_EXT4_PLUGIN)
  string(APPEND XV_FS_ENTRIES "file\t0644\t/etc/fsplugins/ext4.conf\t${CMAKE_CURRENT_SOURCE_DIR}/fs/plugins/ext4/ext4.conf\n")
else()
  set(XV_FS_ENTRIES "${core_fs_entries}")
  set(XV_USER_BINARIES "${core_user_binaries}")
endif()
set(boot_bundle "${PROJECT_BINARY_DIR}/user/image/_fsboot")
add_custom_command(OUTPUT "${boot_bundle}"
  COMMAND "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tools/pack_boot_assets.py"
    --base "${PROJECT_BINARY_DIR}/user/image/_fsboot.base" --output "${boot_bundle}"
    ${plugin_bundle_arguments}
  DEPENDS user_fsboot_executable "${PROJECT_BINARY_DIR}/user/image/_fsboot.base"
    ${plugin_bundle_dependencies} "${PROJECT_SOURCE_DIR}/tools/pack_boot_assets.py"
  VERBATIM)
add_custom_target(user_fsboot DEPENDS "${boot_bundle}")
list(APPEND XV_FS_ASSETS "${boot_bundle}" ${plugin_bundle_dependencies})
set(XV_USER_BINARIES ${XV_USER_BINARIES} PARENT_SCOPE)
set(XV_FS_ENTRIES "${XV_FS_ENTRIES}" PARENT_SCOPE)
set(XV_FS_ASSETS ${XV_FS_ASSETS} PARENT_SCOPE)
