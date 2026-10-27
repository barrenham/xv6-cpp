#include "kernel/support/types.h"
#include "user/runtime/user.h"
#include "user/runtime/accounts.h"
extern "C" int main(int argc, char **argv)
{
  if (argc != 2) { fprintf(2, "usage: useradd NAME\n"); exit(1); }
  int uid = account::add(argv[1]);
  if (uid < 0) { fprintf(2, "useradd: root required, duplicate user, busy database or creation failed\n"); exit(1); }
  printf("useradd: %s uid=%d gid=%d\n", argv[1], uid, uid); exit(0);
}
