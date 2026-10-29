#include "kernel/support/types.h"
#include "user/runtime/user.h"

static void check(bool value, const char *message)
{
  if (!value) { fprintf(2, "schedulercheck: %s\n", message); exit(1); }
}
static void receive(int fd, void *destination, int bytes)
{
  auto *out = static_cast<char *>(destination);
  while (bytes)
  {
    int count = read(fd, out, bytes);
    check(count > 0, "pipe receive");
    out += count; bytes -= count;
  }
}
struct Result { unsigned index; uint64 work; };
extern "C" int main(int argc, char **argv)
{
  int workers = argc == 2 ? atoi(argv[1]) : 3;
  check(workers >= 3 && workers <= 16, "worker count");
  int ready[2], gate[2], results[2];
  check(pipe(ready) == 0 && pipe(gate) == 0 && pipe(results) == 0, "pipes");
  for (int i = 0; i < workers; ++i)
  {
    int pid = fork(); check(pid >= 0, "create scheduling worker");
    if (pid == 0)
    {
      close(ready[0]); close(gate[1]); close(results[0]);
      check(write(ready[1], "r", 1) == 1, "ready notification");
      close(ready[1]);
      uint64 start;
      receive(gate[0], &start, sizeof(start)); close(gate[0]);
      while (clockticks() < start) {}
      uint64 end = start + 30000000; // Three seconds, not fork timing.
      volatile uint64 accumulator = i + 1;
      uint64 work = 0;
      while (clockticks() < end)
      {
        for (unsigned j = 0; j < 10000; ++j)
          accumulator = accumulator * 6364136223846793005UL + 1;
        work += 10000;
      }
      Result result{static_cast<unsigned>(i), work};
      check(write(results[1], &result, sizeof(result)) == sizeof(result), "result notification");
      close(results[1]); exit(0);
    }
  }
  close(ready[1]); close(gate[0]); close(results[1]);
  char token;
  for (int i = 0; i < workers; ++i) receive(ready[0], &token, 1);
  close(ready[0]);
  uint64 start = clockticks() + 5000000;
  for (int i = 0; i < workers; ++i)
    check(write(gate[1], &start, sizeof(start)) == sizeof(start), "release barrier");
  close(gate[1]);
  unsigned seen = 0;
  uint64 minimum = ~uint64(0), maximum = 0;
  for (int i = 0; i < workers; ++i)
  {
    Result result;
    receive(results[0], &result, sizeof(result));
    check(result.index < unsigned(workers) && !(seen & (1U << result.index)), "unique worker");
    seen |= 1U << result.index;
    check(result.work != 0, "runnable worker starved");
    if (result.work < minimum) minimum = result.work;
    if (result.work > maximum) maximum = result.work;
  }
  close(results[0]);
  for (int i = 0; i < workers; ++i)
  {
    int status = -1;
    check(wait(&status) > 0 && status == 0, "worker exit and reap");
  }
  // A coarse integration bound, not a throughput benchmark or proof of exact
  // fair shares. Deterministic host contracts test weighted CPU accounting.
  check(maximum <= minimum * 3, "CPU progress diverged beyond coarse scheduling bound");
  printf("schedulercheck: all workers made bounded progress\n");
  exit(0);
}
