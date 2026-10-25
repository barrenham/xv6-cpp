#include "kernel/support/types.h"
#include "user/runtime/user.h"

static void check(bool ok, const char *message)
{
  if (!ok)
  {
    fprintf(2, "grouptest: FAIL: %s\n", message);
    exit(1);
  }
}

static void send_int(int fd, int value)
{
  check(write(fd, &value, sizeof(value)) == sizeof(value), "pipe send");
}

static int receive_int(int fd)
{
  int value = -1;
  check(read(fd, &value, sizeof(value)) == sizeof(value), "pipe receive");
  return value;
}

static void wait_ok(int pid)
{
  int status = -1;
  check(wait(&status) == pid && status == 0, "child status");
}

static void decimal(int value, char *output)
{
  char reverse[16];
  int count = 0;
  do
  {
    reverse[count++] = '0' + value % 10;
    value /= 10;
  } while (value);
  for (int i = 0; i < count; ++i)
  {
    output[i] = reverse[count - i - 1];
  }
  output[count] = 0;
}

static void exec_rules()
{
  int ready[2], gate[2];
  check(pipe(ready) == 0 && pipe(gate) == 0, "exec pipes");
  int sid = getsid(0);
  int child = fork();
  check(child >= 0, "exec fork");
  if (child == 0)
  {
    close(ready[0]);
    close(gate[1]);
    char *bad[] = {const_cast<char *>("absent"), nullptr};
    check(exec("/absent-group-test", bad) == -1, "failed exec");
    send_int(ready[1], 1);
    receive_int(gate[0]);
    char sid_text[16], group_text[16], ready_text[16], gate_text[16];
    decimal(sid, sid_text);
    decimal(getpid(), group_text);
    decimal(ready[1], ready_text);
    decimal(gate[0], gate_text);
    char *args[] = {const_cast<char *>("grouptest"),
                    const_cast<char *>("exec"),
                    sid_text,
                    group_text,
                    ready_text,
                    gate_text,
                    nullptr};
    exec("grouptest", args);
    check(false, "exec helper");
  }
  close(ready[1]);
  close(gate[0]);
  check(receive_int(ready[0]) == 1, "pre-exec ready");
  check(setpgid(child, 0) == 0, "parent setpgid after failed exec");
  send_int(gate[1], 1);
  check(receive_int(ready[0]) == 2, "successful exec ready");
  check(setpgid(child, getpgrp()) == -1, "parent setpgid rejected after exec");
  send_int(gate[1], 1);
  close(ready[0]);
  close(gate[1]);
  wait_ok(child);
  check(getsid(child) == -1, "reaped PID no longer queryable");
}

static void sessions()
{
  int ready[2], gate[2];
  check(pipe(ready) == 0 && pipe(gate) == 0, "session pipes");
  int inherited_group = getpgrp();
  int inherited_session = getsid(0);
  int child = fork();
  check(child >= 0, "session fork");
  if (child == 0)
  {
    close(ready[0]);
    close(gate[1]);
    check(getpgrp() == inherited_group && getsid(0) == inherited_session,
          "fork inherits identities");
    check(setpgid(0, 0) == 0 && getpgrp() == getpid(), "create own group");
    check(setsid() == -1, "group leader cannot setsid");
    check(setpgid(0, inherited_group) == 0, "rejoin existing group");
    check(setpgid(0, 0x7fffffff) == -1, "nonexistent group rejected");
    check(setsid() == getpid(), "create session");
    check(getsid(0) == getpid() && getpgrp() == getpid(), "session leader identity");
    check(setsid() == -1 && setpgid(0, 0) == -1, "session leader restrictions");
    send_int(ready[1], getpid());
    receive_int(gate[0]);
    int worker = fork();
    check(worker >= 0, "new session worker");
    if (worker == 0)
    {
      check(setpgid(0, inherited_group) == -1, "cross-session group rejected");
      check(setpgid(0, 0) == 0 && getsid(0) != getpid(), "group within session");
      exit(0);
    }
    wait_ok(worker);
    exit(0);
  }
  close(ready[1]);
  close(gate[0]);
  check(receive_int(ready[0]) == child && getsid(child) == child,
        "query another session");
  check(setpgid(child, inherited_group) == -1,
        "parent cannot move child across sessions");
  send_int(gate[1], 1);
  close(ready[0]);
  close(gate[1]);
  wait_ok(child);
  check(getpgrp() == inherited_group && getsid(0) == inherited_session,
        "child changes leave parent unchanged");
}

static void leader_exit(bool new_session)
{
  int ready[2], gate[2];
  check(pipe(ready) == 0 && pipe(gate) == 0, "leader lifetime pipes");
  int original_session = getsid(0);
  int leader = fork();
  check(leader >= 0, "leader fork");
  if (leader == 0)
  {
    close(ready[0]);
    close(gate[1]);
    check(new_session ? setsid() == getpid() : setpgid(0, 0) == 0, "leader identity");
    int group = getpgrp();
    int session = getsid(0);
    int survivor = fork();
    check(survivor >= 0, "surviving member fork");
    if (survivor == 0)
    {
      receive_int(gate[0]);
      check(getpgrp() == group && getsid(0) == session,
            "identity survives leader reaping");
      check(setpgid(0, 0) == 0 && getsid(0) == session,
            "last member moves groups without losing session");
      send_int(ready[1], 1);
      exit(0);
    }
    send_int(ready[1], survivor);
    exit(0);
  }
  close(ready[1]);
  close(gate[0]);
  int survivor = receive_int(ready[0]);
  wait_ok(leader);
  check(getsid(leader) == -1 &&
            getsid(survivor) == (new_session ? leader : original_session),
        "surviving member is discoverable");
  check(setpgid(survivor, 0) == -1, "grandchild adopted by init is not our child");
  send_int(gate[1], 1);
  check(receive_int(ready[0]) == 1, "survivor completed");
  close(ready[0]);
  close(gate[1]);
}

extern "C" int main(int argc, char **argv)
{
  if (argc == 6 && strcmp(argv[1], "exec") == 0)
  {
    check(getsid(0) == atoi(argv[2]) && getpgrp() == atoi(argv[3]),
          "exec preserves identities");
    check(setpgid(0, 0) == 0, "self setpgid allowed after exec");
    send_int(atoi(argv[4]), 2);
    receive_int(atoi(argv[5]));
    return 0;
  }
  check(getpgrp() > 0 && getsid(0) == getsid(getpid()), "initial identity");
  check(getsid(-1) == -1 && getsid(0x7fffffff) == -1 && setpgid(-1, 0) == -1 &&
            setpgid(0, -1) == -1 && setpgid(0x7fffffff, 0) == -1,
        "invalid IDs");
  exec_rules();
  sessions();
  leader_exit(false);
  leader_exit(true);
  for (int round = 0; round < 24; ++round)
  {
    int child = fork();
    check(child >= 0, "reuse fork");
    if (child == 0)
    {
      check(setsid() == getpid(), "repeated session creation");
      exit(0);
    }
    wait_ok(child);
    check(getsid(child) == -1, "repeated membership retirement");
  }
  printf("grouptest: all tests succeeded\n");
  return 0;
}
