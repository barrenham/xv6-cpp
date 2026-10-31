#include "kernel/support/types.h"
#include "kernel/fs/stat.h"
#include "kernel/fs/fcntl.h"
#include "kernel/fs/poll.h"
#include "kernel/tty/termios.h"
#include "kernel/process/signals/signal.h"
#include "user/runtime/user.h"
namespace
{
void check(bool ok, const char *message)
{ if (!ok) { fprintf(2, "namedptytest: FAIL: %s\n", message); exit(1); } }
void raw(int fd)
{ termios value; check(tcgetattr(fd, &value) == 0, "get termios"); cfmakeraw(&value); check(tcsetattr(fd, TCSANOW, &value) == 0, "raw mode"); }
void controlling_race()
{
  for(unsigned round=0;round<8;++round)
  {
    int master=posix_openpt(O_RDWR),gate[2],done[2];char path[32];
    check(master>=0 && unlockpt(master)==0 && ptsname_r(master,path,sizeof(path))==0 && pipe(gate)==0 && pipe(done)==0,"controlling race fixtures");
    int slave=open(path,O_RDWR);check(slave>=0,"controlling race slave");
    int child=fork();check(child>=0,"controlling race fork");
    if(!child)
    {
      close(master);close(gate[0]);close(done[1]);
      check(setsid()==getpid() && signal(SIGHUP,SIG_IGN)!=SIG_ERR && ioctl(slave,TIOCSCTTY,0)==0,"controlling race session");
      close(slave);char value;check(write(gate[1],"r",1)==1,"publish controlling race");
      for(unsigned attempt=0;attempt<1000;++attempt)
      {int tty=open("/dev/tty",O_RDWR);if(tty<0)break;close(tty);}
      check(read(done[0],&value,1)==1 && open("/dev/tty",O_RDWR)<0,"controlling race eventually detached");
      close(gate[1]);close(done[0]);exit(0);
    }
    close(slave);close(gate[1]);close(done[0]);char value;
    check(read(gate[0],&value,1)==1,"wait controlling race session");
    close(master);check(write(done[1],"h",1)==1,"controlling race master hangup");close(gate[0]);close(done[1]);
    int status;check(wait(&status)==child && status==0,"controlling race lifetime");
  }
}
void controlling_terminal()
{
  int pair[2],ready[2],resume[2];
  check(openpty(pair)==0 && pipe(ready)==0 && pipe(resume)==0,"controlling tty fixtures");
  int child=fork();check(child>=0,"controlling tty fork");
  if(!child)
  {
    close(pair[0]);close(ready[0]);close(resume[1]);
    check(setsid()==getpid() && open("/dev/tty",O_RDWR)<0,"new session initially has no controlling tty");
    check(signal(SIGHUP,SIG_IGN)!=SIG_ERR && ioctl(pair[1],TIOCSCTTY,0)==0,"claim controlling slave");
    raw(pair[1]);int tty=open("/dev/tty",O_RDWR);check(tty>=0,"open controlling slave alias");
    check(tcgetpgrp(tty)==getpgrp(),"controlling alias preserves foreground session");close(pair[1]);
    check(write(tty,"alias",5)==5,"controlling alias survives original slave close");
    char value;check(read(tty,&value,1)==1 && value=='R',"controlling alias receives master input");
    check(write(ready[1],"r",1)==1 && read(resume[0],&value,1)==1,"wait for master hangup");
    check(open("/dev/tty",O_RDWR)<0 && read(tty,&value,1)==0,"hangup detaches new opens and drains old alias");
    close(tty);close(ready[1]);close(resume[0]);exit(0);
  }
  close(pair[1]);close(ready[1]);close(resume[0]);
  char bytes[5],value;unsigned got=0;
  while(got<sizeof(bytes)){int n=read(pair[0],bytes+got,sizeof(bytes)-got);check(n>0,"controlling alias output");got+=n;}
  check(!memcmp(bytes,"alias",5) && write(pair[0],"R",1)==1 && read(ready[0],&value,1)==1,"controlling roundtrip");
  close(pair[0]);check(write(resume[1],"h",1)==1,"publish controlling hangup");close(resume[1]);close(ready[0]);
  int status;check(wait(&status)==child && status==0,"controlling terminal child exits cleanly");
}
}
extern "C" int main()
{
  controlling_terminal();controlling_race();
  char path[32], other_path[32], data[16];
  int master = posix_openpt(O_RDWR | O_NONBLOCK); check(master >= 0, "allocate master");
  check(grantpt(master) == 0 && ptsname_r(master, path, sizeof(path)) == 0, "slave name");
  check(ptsname_r(master, data, 2) == -1, "short name buffer");
  check(open(path, O_RDWR) < 0, "slave initially locked");
  check(ioctl(master, TIOCGPTN, ~0UL) == -1, "bad ioctl pointer");
  check(unlockpt(master) == 0, "unlock slave");
  int slave = open(path, O_RDWR), another = open(path, O_RDWR); check(slave >= 0 && another >= 0, "multiple slave opens");
  raw(slave);
  check(write(master, "abc", 3) == 3 && read(slave, data, 3) == 3 && !memcmp(data, "abc", 3), "master to slave");
  close(slave);
  check(write(another, "xyz", 3) == 3 && read(master, data, 3) == 3 && !memcmp(data, "xyz", 3), "one slave close preserves peer");
  close(another);
  slave = open(path, O_RDWR); check(slave >= 0, "reopen slave after last close");
  check(write(master, "r", 1) == 1 && read(slave, data, 1) == 1, "reopened terminal works");
  int alias = dup(master); check(alias >= 0, "duplicate master"); close(master);
  check(write(slave, "d", 1) == 1 && read(alias, data, 1) == 1 && data[0] == 'd', "dup master lifetime");
  close(alias);
  struct stat value; check(stat(path, &value) < 0 && open(path, O_RDWR) < 0, "master close removes slave name");
  check(read(slave, data, 1) == 0, "slave EOF after master close");
  int other = posix_openpt(O_RDWR); check(other >= 0 && ptsname_r(other, other_path, sizeof(other_path)) == 0 && strcmp(path, other_path), "old slave pins number");
  close(other); close(slave);
  master = posix_openpt(O_RDWR); check(master >= 0 && ptsname_r(master, other_path, sizeof(other_path)) == 0 && !strcmp(path, other_path), "number reused after final reference");
  close(master);
  for (int i = 0; i < 100; ++i)
  {
    master = posix_openpt(O_RDWR); check(master >= 0 && unlockpt(master) == 0 && ptsname_r(master, path, sizeof(path)) == 0, "repeat allocation");
    slave = open(path, O_RDWR); check(slave >= 0, "repeat slave"); close(slave); close(master);
  }
  int held[16], used = 0;
  while (used < 16)
  {
    int fd = open("/dev/null", O_RDWR);
    if (fd < 0) break;
    held[used++] = fd;
  }
  check(posix_openpt(O_RDWR) < 0, "descriptor exhaustion rolls back");
  while (used) close(held[--used]);
  int workers[4];
  for (int &worker : workers)
  {
    worker = fork(); check(worker >= 0, "concurrent fork");
    if (!worker)
    {
      for (int i = 0; i < 20; ++i)
      {
        int master = posix_openpt(O_RDWR);
        check(master >= 0 && unlockpt(master) == 0 && ptsname_r(master, path, sizeof(path)) == 0, "concurrent master");
        int slave = open(path, O_RDWR); check(slave >= 0, "concurrent slave"); raw(slave);
        check(write(master, "q", 1) == 1 && read(slave, data, 1) == 1 && data[0] == 'q', "independent concurrent pair");
        close(slave); close(master);
      }
      exit(0);
    }
  }
  for (int worker : workers) { (void)worker; int status; check(wait(&status) > 0 && status == 0, "concurrent worker result"); }
  check(posix_openpt(O_RDONLY) < 0 && unlockpt(0) < 0, "wrong descriptor and mode");
  printf("namedptytest: all tests succeeded\n"); exit(0);
}
