#include <cstring>
extern "C" int run_cache_profile();
extern "C" int run_fs_contracts(const char *image);
extern "C" int run_ext3_fs_contracts(const char *image);

int main(int argc, char **argv)
{
  if (argc == 2 && !std::strcmp(argv[1], "--cache-profile")) return run_cache_profile();
  return argc == 2 ? run_fs_contracts(argv[1]) : argc == 3 ? run_ext3_fs_contracts(argv[2]) : 2;
}
