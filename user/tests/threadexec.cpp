#include "user/runtime/user.h"
#include "user/include/pthread.h"
#include "kernel/process/wait/wait.h"

namespace
{
unsigned ready, exec_ready, exec_go, creator_ready;
int channel;
pthread_mutex_t held = PTHREAD_MUTEX_INITIALIZER;
__thread int tls_value = 42;

void
check(bool result, const char *message) {
  if (!result)
  {
    fprintf(2, "threadexec: FAIL: %s\n", message);
    exit(1);
  }
}

void *
writer(void *) {
  __atomic_store_n(&ready, 1U, __ATOMIC_RELEASE);
  for (;;)
  {
    check(write(channel, "W", 1) == 1, "writer pipe");
    pause(1);
  }
}

void *
idle(void *) {
  check(!pthread_mutex_lock(&held), "idle mutex");
  check(!pthread_mutex_unlock(&held), "idle unlock");
  return nullptr;
}

void
decimal(int value, char *output) {
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

[[noreturn]]
void
replacement() {
  char fd[24], pid[24];
  decimal(channel, fd);
  decimal(getpid(), pid);
  char *args[] = {const_cast<char *>("threadexec"), const_cast<char *>("replace"), fd,
                  pid, nullptr};
  exec("/usr/tests/threadexec", args);
  check(false, "valid exec must replace group");
  exit(1);
}

void *
execute_worker(void *argument) {
  if (argument)
  {
    __atomic_add_fetch(&exec_ready, 1U, __ATOMIC_RELEASE);
    while (!__atomic_load_n(&exec_go, __ATOMIC_ACQUIRE))
    {
      sched_yield();
    }
  }
  replacement();
}

void *
short_worker(void *) {
  return nullptr;
}

void *
creating_worker(void *) {
  __atomic_store_n(&creator_ready, 1U, __ATOMIC_RELEASE);
  for (;;)
  {
    pthread_t thread;
    int result = pthread_create(&thread, nullptr, short_worker, nullptr);
    if (!result)
    {
      pthread_join(thread, nullptr);
    }
    else
    {
      sched_yield();
    }
  }
}

void
run(int worker) {
  printf("threadexec: case %d start\n", worker);
  int pipe_fds[2];
  check(!pipe(pipe_fds), "exec result pipe");
  int child = fork();
  check(child >= 0, "fork exec fixture");
  if (!child)
  {
    close(pipe_fds[0]);
    channel = pipe_fds[1];
    pthread_t writer_id, idle_id, exec_id;
    check(!pthread_mutex_lock(&held), "hold mutex across exec");
    check(!pthread_create(&writer_id, nullptr, writer, nullptr) &&
              !pthread_create(&idle_id, nullptr, idle, nullptr),
          "start sibling threads");
    while (!__atomic_load_n(&ready, __ATOMIC_ACQUIRE))
    {
      sched_yield();
    }
    printf("threadexec: siblings ready\n");
    char *bad[] = {const_cast<char *>("missing"), nullptr};
    check(exec("/missing-thread-exec", bad) < 0, "failed exec preserves old process");
    check(!pthread_mutex_unlock(&held) && !pthread_join(idle_id, nullptr),
          "sibling survives failed exec");
    check(!pthread_mutex_lock(&held) &&
              !pthread_create(&idle_id, nullptr, idle, nullptr),
          "sleeping sibling before replacement");
    printf("threadexec: failed exec preserved siblings; replacing\n");
    if (worker)
    {
      if (worker == 2)
      {
        pthread_t competitor, creator;
        check(!pthread_create(&creator, nullptr, creating_worker, nullptr) &&
                  !pthread_create(&exec_id, nullptr, execute_worker,
                                  reinterpret_cast<void *>(1)) &&
                  !pthread_create(&competitor, nullptr, execute_worker,
                                  reinterpret_cast<void *>(1)),
              "concurrent create and competing exec workers");
        while (__atomic_load_n(&exec_ready, __ATOMIC_ACQUIRE) != 2 ||
               !__atomic_load_n(&creator_ready, __ATOMIC_ACQUIRE))
        {
          sched_yield();
        }
        __atomic_store_n(&exec_go, 1U, __ATOMIC_RELEASE);
      }
      else
      {
        check(!pthread_create(&exec_id, nullptr, execute_worker, nullptr),
              "exec from worker");
      }
      pthread_join(exec_id, nullptr);
      check(false, "exec must terminate joining main thread");
    }
    replacement();
  }
  close(pipe_fds[1]);
  bool replaced = false;
  char bytes[32];
  for (;;)
  {
    int count = read(pipe_fds[0], bytes, sizeof(bytes));
    check(count >= 0, "read replacement output");
    if (!count)
    {
      break;
    }
    for (int i = 0; i < count; ++i)
    {
      check(!replaced, "no old writer survives successful exec");
      if (bytes[i] == 'S')
      {
        replaced = true;
      }
      else
      {
        check(bytes[i] == 'W', "expected old writer output");
      }
    }
  }
  int status;
  check(replaced && waitpid(child, &status, 0) == child && WIFEXITED(status) &&
            !WEXITSTATUS(status),
        "successful exec preserves PID and normal process lifetime");
  close(pipe_fds[0]);
}
} // namespace

extern "C" int
main(int argc, char **argv) {
  if (argc == 4 && !strcmp(argv[1], "replace"))
  {
    check(getpid() == atoi(argv[3]) && pthread_self() && tls_value == 42 && errno == 0,
          "replacement PID and fresh runtime/TLS");
    int fd = atoi(argv[2]);
    check(write(fd, "S", 1) == 1, "replacement marker");
    pause(3);
    return 0;
  }
  run(0);
  run(1);
  run(2);
  printf(
      "threadexec: PASS: failed exec preserves siblings, main/worker/competing exec stops group and preserves PID\n");
  return 0;
}
