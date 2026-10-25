#include "kernel/support/types.h"
#include "user/runtime/user.h"

namespace
{
constexpr int Children = 96;

void fail(const char *message)
{
  fprintf(2, "procext: %s\n", message);
  exit(1);
}

void round(bool terminate)
{
  int gate[2];
  int pids[Children];
  if (pipe(gate) < 0)
  {
    fail("pipe");
  }
  for (int i = 0; i < Children; ++i)
  {
    int pid = fork();
    if (pid < 0)
    {
      fail("could not grow beyond the old process limit");
    }
    if (pid == 0)
    {
      close(gate[1]);
      char token;
      int count = read(gate[0], &token, 1);
      close(gate[0]);
      exit(count == 1 ? 0 : 1);
    }
    pids[i] = pid;
  }
  close(gate[0]);
  if (terminate)
  {
    pause(2);
    for (int i = 0; i < Children; ++i)
    {
      if (kill(pids[i]) < 0)
      {
        fail("kill blocked child");
      }
    }
  }
  else
  {
    char tokens[Children]{};
    if (write(gate[1], tokens, sizeof(tokens)) != sizeof(tokens))
    {
      fail("wake blocked children");
    }
  }
  close(gate[1]);
  for (int i = 0; i < Children; ++i)
  {
    int status;
    int pid = wait(&status);
    if (pid < 0 || status != (terminate ? -1 : 0))
    {
      fail("reap child status");
    }
    bool found = false;
    for (int j = 0; j < Children; ++j)
    {
      if (pids[j] == pid)
      {
        pids[j] = 0;
        found = true;
        break;
      }
    }
    if (!found)
    {
      fail("duplicate or unknown child");
    }
  }
  if (wait(nullptr) != -1)
  {
    fail("unreaped child");
  }
}
} // namespace

extern "C" int main()
{
  round(false);
  round(true);
  round(false);
  printf("procext: 96 concurrent children, wake/kill/reuse OK\n");
  exit(0);
}
