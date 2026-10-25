#include <cstring>
extern "C" int run_process_group_contracts();
extern "C" int run_signal_contracts();

int main(int argc, char **argv)
{
  if (argc == 2 && std::strcmp(argv[1], "--signals") == 0)
  {
    return run_signal_contracts();
  }
  return run_process_group_contracts();
}
