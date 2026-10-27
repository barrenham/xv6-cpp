#include "kernel/support/types.h"
#include "user/runtime/user.h"
#include "user/runtime/accounts.h"
extern "C" int main(int argc, char **argv)
{
  if (argc < 3) { fprintf(2, "usage: chown USER[:GROUP] FILE...\n"); exit(1); }
  char *group = strchr(argv[1], ':'); if (group) *group++ = 0;
  unsigned uid, gid = 0; account::User user{};
  if (!account::numeric(argv[1], uid)) { if (!account::find(argv[1], user)) exit(1); uid = user.uid; }
  if (group && !account::numeric(group, gid)) { if (!account::find(group, user)) exit(1); gid = user.gid; }
  int failed = 0;
  for (int i = 2; i < argc; ++i) if (chown(argv[i], uid, group ? static_cast<int>(gid) : -1) < 0)
  { fprintf(2, "chown: %s failed\n", argv[i]); failed = 1; }
  exit(failed);
}
