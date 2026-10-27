#include "kernel/fs/image_paths.h"
#include "kernel/support/types.h"
#include "user/runtime/user.h"
#include "user/runtime/accounts.h"
extern "C" int main(int argc, char **argv)
{
  if (geteuid() != 0) { fprintf(2, "su: root required\n"); exit(1); }
  account::User user{};
  if (!account::find(argc > 1 ? argv[1] : "root", user)) { fprintf(2, "su: unknown user\n"); exit(1); }
  for (int fd = 3; fd < 16; ++fd) close(fd);
  clearenv(); setenv("PATH", XV_DEFAULT_PATH, 1); setenv("HOME", user.home, 1); setenv("USER", user.name, 1); setenv("SHELL", user.shell, 1);
  umask(0022);
  unsigned extra[16]; int count = account::groups(user, extra);
  if (count < 0 || setgroups(count, extra) < 0 || setgid(user.gid) < 0 || setuid(user.uid) < 0 || chdir(user.home) < 0)
  { fprintf(2, "su: credential switch failed\n"); exit(1); }
  char *shell_arguments[] = {user.shell, nullptr};
  if (argc > 2) execvp(argv[2], argv + 2); else execv(user.shell, shell_arguments);
  fprintf(2, "su: exec failed\n"); exit(1);
}
