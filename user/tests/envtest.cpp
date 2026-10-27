#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "user/runtime/user.h"
namespace
{
void check(bool ok, const char *message) { if (!ok) { fprintf(2, "envtest: FAIL: %s\n", message); exit(1); } }
void reap() { int status; check(wait(&status) > 0 && status == 0, "child result"); }
}
extern "C" int main(int argc, char **argv)
{
  if (argc > 1)
  {
    if (!strcmp(argv[1], "inherited")) check(getenv("KEY") && !strcmp(getenv("KEY"), "child"), "exec inherits environment");
    if (!strcmp(argv[1], "explicit")) check(getenv("ONLY") && !strcmp(getenv("ONLY"), "explicit") && !getenv("KEY") &&
      !strcmp(getenv("SPACES"), "two words"), "execve explicit environment");
    if (!strcmp(argv[1], "empty")) check(environ && !environ[0], "empty environment");
    exit(0);
  }
  check(setenv("KEY", "parent", 1) == 0 && setenv("KEY", "ignored", 0) == 0 && !strcmp(getenv("KEY"), "parent"), "set and overwrite");
  check(setenv("", "bad", 1) < 0 && setenv("A=B", "bad", 1) < 0 && unsetenv("A=B") < 0, "invalid names");
  const char *phases[] = {"inherited", "explicit", "empty"};
  for (const char *phase : phases)
  {
    int child = fork(); check(child >= 0, "fork");
    if (!child)
    {
      check(setenv("KEY", "child", 1) == 0, "child change");
      char *args[] = {const_cast<char *>("envtest"), const_cast<char *>(phase), nullptr};
      if (!strcmp(phase, "inherited")) { setenv("PATH", "/missing:/usr/tests", 1); execvp("envtest", args); }
      else if (!strcmp(phase, "explicit"))
      { char *values[] = {const_cast<char *>("ONLY=explicit"), const_cast<char *>("SPACES=two words"), nullptr}; execve("/usr/tests/envtest", args, values); }
      else execve("/usr/tests/envtest", args, nullptr);
      check(false, "exec failed");
    }
    reap(); check(!strcmp(getenv("KEY"), "parent"), "fork mutation isolated");
  }
  char *args[] = {const_cast<char *>("envtest"), nullptr};
  check(execve("/usr/tests/envtest", args, reinterpret_cast<char *const *>(~0UL)) < 0 && !strcmp(getenv("KEY"), "parent"), "bad env pointer preserves image");
  char *too_many[MAXENV + 1]; for (unsigned i = 0; i < MAXENV; ++i) too_many[i] = const_cast<char *>("X=y"); too_many[MAXENV] = nullptr;
  check(execve("/usr/tests/envtest", args, too_many) < 0, "environment count bound");
  check(unsetenv("KEY") == 0 && !getenv("KEY") && clearenv() == 0 && !environ[0], "unset and clear");
  check(setenv("AFTER", "clear", 1) == 0 && !strcmp(getenv("AFTER"), "clear"), "set after clear");
  printf("envtest: all tests succeeded\n"); exit(0);
}
