#include "kernel/fs/image_paths.h"
// init: The initial user-level program

#include "kernel/support/types.h"
#include "kernel/fs/stat.h"
#include "user/runtime/user.h"
#include "kernel/fs/fcntl.h"
#include "user/runtime/accounts.h"
#include "kernel/tty/termios.h"

extern "C"
{

  char *argv[] = {"login", 0};

  int main(void)
  {
    int pid, wpid;
    setenv("PATH", XV_DEFAULT_PATH, 0);
    setenv("HOME", "/root", 0);
    setenv("USER", "root", 0);
    setenv("SHELL", XV_SHELL_PATH, 0);

    if (open("/dev/console", O_RDWR) != 0)
    {
      exit(1);
    }
    dup(0); // stdout
    dup(0); // stderr
    if (!account::initialize())
    {
      fprintf(2, "init: account setup failed\n");
      exit(1);
    }

    int state = account::password_state();
    if (state < 0)
    {
      fprintf(2, "init: invalid shadow database\n");
      exit(1);
    }
    if (!state)
    {
      printf("First boot: set the root password (8..63 bytes).\n");
      while (!account::change_password("root"))
      {
      }
    }
    termios settings{};
    bool terminal = tcgetattr(0, &settings) == 0;
    for (;;)
    {
      if (terminal)
      {
        tcsetattr(0, TCSAFLUSH, &settings);
      }
      printf("init: starting login\n");
      pid = fork();
      if (pid < 0)
      {
        printf("init: fork failed\n");
        exit(1);
      }
      if (pid == 0)
      {
        exec("/bin/login", argv);
        printf("init: exec login failed\n");
        exit(1);
      }

      for (;;)
      {
        // wait() returns if the login shell exits,
        // or if a parentless process exits.
        wpid = wait((int *)0);
        if (wpid == pid)
        {
          // The login shell exited; return to the login screen.
          break;
        }
        else if (wpid < 0)
        {
          printf("init: wait returned an error\n");
          exit(1);
        }
        else
        {
          // it was a parentless process; do nothing.
        }
      }
    }
  }
}
