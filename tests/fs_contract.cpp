extern "C" int run_fs_contracts(const char *image);

int main(int argc, char **argv)
{
  return argc == 2 ? run_fs_contracts(argv[1]) : 2;
}
