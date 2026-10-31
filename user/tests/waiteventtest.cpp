#include "kernel/support/types.h"
#include "kernel/process/signals/signal.h"
#include "kernel/process/wait/wait.h"
#include "user/runtime/user.h"

namespace
{
void check(bool value, const char *message)
{
  if (!value) { fprintf(2,"waiteventtest: FAIL: %s\n",message);exit(1); }
}
constexpr unsigned Children = 160;
int children[Children];
}
extern "C" int main()
{
  int gate[2], ready[2];
  check(!pipe(gate) && !pipe(ready),"pipes");
  for (unsigned i = 0; i < Children; ++i)
  {
    int pid = fork();
    check(pid >= 0,"fork live child population");
    if (!pid)
    {
      close(gate[1]);close(ready[0]);
      check(write(ready[1],"r",1)==1,"publish ready");
      char byte;
      (void)read(gate[0],&byte,1);
      exit(17);
    }
    children[i] = pid;
  }
  close(gate[0]);close(ready[1]);
  for (unsigned i = 0; i < Children; ++i)
  { char byte;check(read(ready[0],&byte,1)==1,"wait child readiness"); }
  close(ready[0]);
  int status = 0x1234;
  for (unsigned i = 0; i < 100; ++i)
    check(!waitpid(-1,&status,WNOHANG) && status==0x1234,"no pending event leaves status unchanged");
  check(waitpid(getpid(),&status,WNOHANG)==-1,"indexed PID must be a direct child");
  // Leave the oldest child alive; the newest exits first. Selection is based on
  // published events rather than walking the whole live-child population.
  int selected = children[Children-1];
  check(!sigsend(selected,SIGTERM),"target indexed PID among live children");
  check(waitpid(-1,reinterpret_cast<int *>(~0UL),0)==-1,"failed copyout preserves exit event");
  check(waitpid(-1,&status,0)==selected && WIFSIGNALED(status) && WTERMSIG(status)==SIGTERM,
        "any-child event selection and preserved status");
  check(sigsend(selected,0)==-1 && waitpid(selected,&status,0)==-1,"reaped PID no longer visible");
  selected=children[Children/2];
  check(!setpgid(selected,selected),"child group assignment");
  check(!waitpid(-selected,&status,WNOHANG),"live group has no event");
  check(waitpid(-2000000000,&status,WNOHANG)==-1,"absent child group");
  for (unsigned repeat=0; repeat<8; ++repeat)
  {
    check(!sigsend(selected,SIGSTOP),"stop child");
    check(waitpid(repeat%2 ? -selected : -1,&status,WUNTRACED)==selected && WIFSTOPPED(status),"queued stop selected among live children");
    check(!waitpid(-1,&status,WNOHANG|WUNTRACED),"stop event consumed exactly once");
    check(!sigsend(selected,SIGCONT),"continue child");
    check(waitpid(repeat%2 ? -selected : -1,&status,WCONTINUED)==selected && WIFCONTINUED(status),"deferred continue queued and wakes parent");
    check(!waitpid(-1,&status,WNOHANG|WCONTINUED),"continue event consumed exactly once");
  }
  // EOF releases all remaining children together, stressing concurrent producers
  // and targeted wakeups while the parent drains its exit queue.
  close(gate[1]);
  unsigned reaped=1;
  while (waitpid(-1,&status,0)>0)
  { check(WIFEXITED(status) && WEXITSTATUS(status)==17,"normal queued exit status");++reaped; }
  check(reaped==Children,"every child reaped exactly once");
  // Reparent children with each kind of unconsumed event. The original parent
  // must not retain event links, and init must reap the final orphan exit.
  for (unsigned mode=0; mode<3; ++mode)
  {
    int report[2];check(!pipe(report),"orphan report pipe");
    int parent=fork();check(parent>=0,"orphan parent");
    if (!parent)
    {
      close(report[0]);
      int child=fork();check(child>=0,"orphan child");
      if (!child)
      {
        close(report[1]);
        if (!mode) exit(23);
        for (;;) pause(1);
      }
      if (mode) check(!sigsend(child,SIGSTOP),"orphan stop");
      check(waitpid(child,reinterpret_cast<int *>(~0UL),mode ? WUNTRACED : 0)==-1,
            "preserve event before adoption");
      if (mode==2)
      {
        check(!sigsend(child,SIGCONT),"orphan continue");
        check(waitpid(child,reinterpret_cast<int *>(~0UL),WCONTINUED)==-1,
              "preserve continue before adoption");
      }
      check(write(report[1],&child,sizeof(child))==sizeof(child),"report orphan PID");
      exit(0);
    }
    close(report[1]);int orphan=0;
    check(read(report[0],&orphan,sizeof(orphan))==sizeof(orphan),"receive orphan PID");
    close(report[0]);
    check(waitpid(parent,&status,0)==parent && WIFEXITED(status) && !WEXITSTATUS(status),
          "original parent releases all child event links");
    if (mode) check(!sigsend(orphan,SIGKILL),"terminate adopted child");
    unsigned retries=0;
    while (!sigsend(orphan,0) && retries++<50) pause(1);
    check(sigsend(orphan,0)==-1,"init reaps adopted event child");
  }
  printf("waiteventtest: ALL PASSED (%u children)\n",Children);
  exit(0);
}
