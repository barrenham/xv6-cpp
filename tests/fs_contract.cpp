#include <cstring>
extern "C" int run_cache_profile();
extern "C" int run_fs_contracts(const char *);

int
main(int argc, char **argv) {
  if (argc == 2 && !std::strcmp(argv[1], "--cache-profile"))
  {
    return run_cache_profile();
  }
  return run_fs_contracts(nullptr);
}
