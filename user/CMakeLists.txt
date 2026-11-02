find_program(XV_USER_OBJCOPY llvm-objcopy HINTS "$ENV{CONDA_PREFIX}/bin" REQUIRED)
add_library(user_runtime OBJECT runtime/ulib.cpp runtime/path.cpp runtime/file_copy.cpp runtime/environment.cpp runtime/pty.cpp runtime/signals.cpp runtime/termios.cpp runtime/printf.cpp runtime/umalloc.cpp runtime/usys.S runtime/start.S)
target_link_libraries(user_runtime PRIVATE xv_options)
add_library(user_fs STATIC runtime/userfs/server.cpp runtime/userfs/block_io.cpp
  runtime/userfs/ext3.cpp runtime/userfs/devices.cpp
  runtime/userfs/memory.cpp runtime/userfs/memory_heap.cpp
  runtime/userfs/processes.cpp runtime/userfs/process_system.cpp
  runtime/userfs/xv6_volume.cpp runtime/userfs/xv6.cpp ../kernel/fs/ext3/volume.cpp ../kernel/fs/ext3/mutate.cpp
  ../kernel/fs/ext3/jbd.cpp)
target_link_libraries(user_fs PRIVATE xv_options)
add_library(user_accounts STATIC runtime/accounts.cpp runtime/account_admin.cpp runtime/authentication.cpp)
target_link_libraries(user_accounts PRIVATE xv_options)
add_library(user_dns STATIC runtime/dns/codec.cpp runtime/dns/resolver.cpp)
target_link_libraries(user_dns PRIVATE xv_options)
add_library(user_http STATIC runtime/http/codec.cpp runtime/http/client.cpp)
target_link_libraries(user_http PRIVATE xv_options PUBLIC user_dns)
function(xv_add_program source)
  get_filename_component(name "${source}" NAME_WE)
  add_executable(user_${name} "${source}" $<TARGET_OBJECTS:user_runtime>)
  target_link_libraries(user_${name} PRIVATE xv_options user_http user_dns user_fs)
  target_link_options(user_${name} PRIVATE -z max-page-size=4096 -e _user_start -T "${PROJECT_SOURCE_DIR}/linker/user.ld")
  set_target_properties(user_${name} PROPERTIES OUTPUT_NAME _${name} RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/user" LINK_DEPENDS "${PROJECT_SOURCE_DIR}/linker/user.ld")
  set(guest_name "${name}")
  if(name STREQUAL "ramfsservicetest")
    set(guest_name "ramfssvctest")
  elseif(name STREQUAL "fsresourcesboottest")
    set(guest_name "fsresboot")
  endif()
  if(source MATCHES "^tests/" OR name STREQUAL "cppdemo" OR name STREQUAL "zombie")
    set(destination "/usr/tests/${guest_name}")
  elseif(name STREQUAL "fsboot" OR name STREQUAL "init" OR name STREQUAL "useradd" OR name STREQUAL "userdel" OR name STREQUAL "groupadd" OR name STREQUAL "groupdel" OR name STREQUAL "usermod" OR name STREQUAL "netctl" OR name STREQUAL "iptables")
    set(destination "/sbin/${name}")
  else()
    set(destination "/bin/${name}")
  endif()
  set(mode 0755)
  if(name STREQUAL "passwd")
    set(mode 04755)
  endif()
  # Guest loaders use program headers; keep full DWARF in the host executable
  # while installing a regular stripped ELF within the legacy xv6 file limit.
  set(guest_binary "${PROJECT_BINARY_DIR}/user/image/_${name}")
  add_custom_command(TARGET user_${name} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${PROJECT_BINARY_DIR}/user/image"
    COMMAND "${XV_USER_OBJCOPY}" --strip-debug $<TARGET_FILE:user_${name}> "${guest_binary}"
    BYPRODUCTS "${guest_binary}" VERBATIM)
  set(XV_FS_ENTRIES "${XV_FS_ENTRIES}file\t${mode}\t${destination}\t${guest_binary}\n" PARENT_SCOPE)
  set(XV_USER_BINARIES ${XV_USER_BINARIES} $<TARGET_FILE:user_${name}> "${guest_binary}" PARENT_SCOPE)
endfunction()
set(programs
  apps/fsboot/fsboot.cpp
  apps/ramfsd/ramfsd.cpp
  apps/memfsd/memfsd.cpp
  apps/tarfsd/tarfsd.cpp
  apps/fbterm/fbterm.cpp
  apps/fbdemo/fbdemo.cpp
  apps/userdel/userdel.cpp
  apps/groupadd/groupadd.cpp
  apps/groupdel/groupdel.cpp
  apps/usermod/usermod.cpp
  apps/readlink/readlink.cpp
  apps/pwd/pwd.cpp
  apps/cp/cp.cpp
  apps/mv/mv.cpp
  apps/login/login.cpp
  apps/passwd/passwd.cpp
  apps/mount/mount.cpp
  apps/umount/umount.cpp
  apps/id/id.cpp
  apps/useradd/useradd.cpp
  apps/su/su.cpp
  apps/chmod/chmod.cpp
  apps/chown/chown.cpp
  apps/env/env.cpp
  apps/ps/ps.cpp
  apps/free/free.cpp
  tests/namedptytest.cpp
  tests/procfstest.cpp
  tests/ramfstest.cpp
  tests/ramfsservicetest.cpp
  tests/fsrootboottest.cpp
  tests/xv6fstest.cpp
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
  tests/storagecheck.cpp
  tests/vmcheck.cpp
  tests/workflowtest.cpp
  tests/envtest.cpp
  tests/permissiontest.cpp
  apps/ping/ping.cpp
  apps/clear/clear.cpp
  apps/curl/curl.cpp
  apps/nslookup/nslookup.cpp
  apps/iptables/iptables.cpp
  apps/netctl/netctl.cpp
  apps/cat/cat.cpp
  apps/echo/echo.cpp
  apps/grep/grep.cpp
  apps/init/init.cpp
  apps/kill/kill.cpp
  apps/ln/ln.cpp
  apps/ls/ls.cpp
  apps/mkdir/mkdir.cpp
  apps/rm/rm.cpp
  apps/pty/pty.cpp
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
  apps/sh/sh.cpp
  apps/sync/sync.cpp
  apps/stty/stty.cpp
  apps/wc/wc.cpp
  apps/zombie/zombie.cpp
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
  apps/cppdemo/cppdemo.cpp
)
foreach(source IN LISTS programs)
  xv_add_program("${source}")
endforeach()
set(XV_USER_BINARIES ${XV_USER_BINARIES} PARENT_SCOPE)
string(APPEND XV_FS_ENTRIES "file\t0644\t/usr/share/doc/xv6/fbfont.txt\t${CMAKE_CURRENT_SOURCE_DIR}/apps/fbterm/FONT-LICENSE.txt\n")
set(XV_FS_ENTRIES "${XV_FS_ENTRIES}" PARENT_SCOPE)
set(plugin_sample "${PROJECT_BINARY_DIR}/fs-plugin-sample.tar")
add_custom_command(OUTPUT "${plugin_sample}"
  COMMAND "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tools/make_fs_plugin_sample.py" --output "${plugin_sample}"
  DEPENDS "${PROJECT_SOURCE_DIR}/tools/make_fs_plugin_sample.py" VERBATIM)
add_custom_target(fs-plugin-sample DEPENDS "${plugin_sample}")
string(APPEND XV_FS_ENTRIES "file\t0644\t/usr/share/doc/xv6/plugin.tar\t${plugin_sample}\n")
set(XV_FS_ENTRIES "${XV_FS_ENTRIES}" PARENT_SCOPE)
set(XV_FS_ASSETS "${CMAKE_CURRENT_SOURCE_DIR}/apps/fbterm/FONT-LICENSE.txt" "${plugin_sample}" PARENT_SCOPE)

target_sources(user_sh PRIVATE apps/sh/environment.cpp apps/sh/prompt.cpp apps/sh/editor.cpp)

# Poll contracts reuse the PTY test executable and installed inode.
target_sources(user_ptytest PRIVATE tests/poll_checks.cpp tests/nonblock_checks.cpp)

target_sources(user_signaltest PRIVATE tests/signal_handlers.cpp tests/child_signals.cpp tests/signal_registers.S)

foreach(program init id su useradd chown sh login passwd userdel groupadd groupdel usermod)
  target_link_libraries(user_${program} PRIVATE user_accounts)
endforeach()
