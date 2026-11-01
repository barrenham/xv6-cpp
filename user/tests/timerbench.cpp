#include "kernel/support/types.h"
#include "user/runtime/user.h"

extern "C" int main(int argc, char **argv)
{
  bool idle = argc > 1 && !strcmp(argv[1], "idle");
  if (idle)
  {
    printf("TIMERBENCH idle-start\n");
    poll(nullptr, 0, 3000);
    printf("TIMERBENCH idle-end\n");
    exit(0);
  }
  int intervals[] = {1, 5, 10, 25, 100};
  uint64 elapsed_samples[100]{};
  unsigned index = 0;
  for (int interval : intervals)
  {
    for (int sample = 0; sample < 20; ++sample)
    {
      auto begin = clockticks();
      if (poll(nullptr, 0, interval) != 0)
      {
        exit(1);
      }
      elapsed_samples[index++] = clockticks() - begin;
    }
  }
  printf("TIMERBENCH measured\n");
  index = 0;
  for (int interval : intervals)
  {
    for (int sample = 0; sample < 20; ++sample)
    {
      printf("TIMERBENCH sample %d %lu\n", interval, elapsed_samples[index++]);
    }
  }
  printf("TIMERBENCH done\n");
  exit(0);
}
