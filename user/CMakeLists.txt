add_library(user_runtime OBJECT runtime/ulib.cpp runtime/signals.cpp runtime/termios.cpp runtime/printf.cpp runtime/umalloc.cpp runtime/usys.S runtime/start.S)
target_link_libraries(user_runtime PRIVATE xv_options)
function(xv_add_program source)
  get_filename_component(name "${source}" NAME_WE)
  add_executable(user_${name} "${source}" $<TARGET_OBJECTS:user_runtime>)
  target_link_libraries(user_${name} PRIVATE xv_options)
  target_link_options(user_${name} PRIVATE -z max-page-size=4096 -e _user_start -T "${PROJECT_SOURCE_DIR}/linker/user.ld")
  set_target_properties(user_${name} PROPERTIES OUTPUT_NAME _${name} RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/user" LINK_DEPENDS "${PROJECT_SOURCE_DIR}/linker/user.ld")
  set(XV_USER_BINARIES ${XV_USER_BINARIES} $<TARGET_FILE:user_${name}> PARENT_SCOPE)
endfunction()
set(programs
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
  tests/nettest.cpp
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

# Poll contracts reuse the PTY test executable and installed inode.
target_sources(user_ptytest PRIVATE tests/poll_checks.cpp tests/nonblock_checks.cpp)

target_sources(user_signaltest PRIVATE tests/signal_handlers.cpp tests/child_signals.cpp tests/signal_registers.S)
