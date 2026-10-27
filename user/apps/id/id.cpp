#include "kernel/support/types.h"
#include "user/runtime/user.h"
#include "user/runtime/accounts.h"
extern "C" int main()
{
  account::User user{};
  bool known = account::find(static_cast<unsigned>(getuid()), user);
  printf("uid=%d(%s) gid=%d euid=%d egid=%d", getuid(), known ? user.name : "unknown", getgid(), geteuid(), getegid());
  unsigned groups[16]; int count = getgroups(16, groups);
  printf(" groups=%d", getgid()); for (int i = 0; i < count; ++i) printf(",%d", groups[i]); printf("\n"); exit(0);
}
