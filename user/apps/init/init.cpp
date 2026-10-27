#include "kernel/fs/image_paths.h"
// init: The initial user-level program

#include "kernel/support/types.h"
#include "kernel/fs/stat.h"
#include "user/runtime/user.h"
#include "kernel/fs/fcntl.h"
#include "user/runtime/accounts.h"

extern "C" {

char *argv[] = {"sh", 0};

int
main(void)
{
  int pid, wpid;
  setenv("PATH", XV_DEFAULT_PATH, 0);
  setenv("HOME", "/root", 0);
  setenv("USER", "root", 0);
  setenv("SHELL", XV_SHELL_PATH, 0);

  if (open("/dev/console", O_RDWR) != 0) exit(1);
  dup(0); // stdout
  dup(0); // stderr
  if (!account::initialize()) { fprintf(2, "init: account setup failed\n"); exit(1); }

  for (;;) {
    printf("init: starting sh\n");
    pid = fork();
    if (pid < 0) {
      printf("init: fork failed\n");
      exit(1);
    }
    if (pid == 0) {
      exec(XV_SHELL_PATH, argv);
      printf("init: exec sh failed\n");
      exit(1);
    }

    for (;;) {
      // this call to wait() returns if the shell exits,
      // or if a parentless process exits.
      wpid = wait((int *)0);
      if (wpid == pid) {
        // the shell exited; restart it.
        break;
      } else if (wpid < 0) {
        printf("init: wait returned an error\n");
        exit(1);
      } else {
        // it was a parentless process; do nothing.
      }
    }
  }
}
}
