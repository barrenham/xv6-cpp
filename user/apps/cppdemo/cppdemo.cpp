#include "user/runtime/file.hpp"

using xv::user::File;

// Exercise destruction on early return; 64 iterations exceed the fd limit
// unless the local owner closes its descriptor on every return.
static int read_console() {
  auto console = File::open("console", File::Access::read_only);
  if (!console.valid()) return -1;
  File moved(static_cast<File &&>(console));
  if (console.valid() || !moved.valid()) return -1;
  return 0;
}

extern "C" int main() {
  for (int i = 0; i < 64; ++i)
    if (read_console() != 0) {
      printf("cppdemo: RAII failed\n");
      return 1;
    }
  printf("cppdemo: RAII and move ownership OK\n");
  return 0;
}
