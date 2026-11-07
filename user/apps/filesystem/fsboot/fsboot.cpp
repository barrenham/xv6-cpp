#include "application.h"
import xv.userfs.bootstrap;

extern "C" int
app_main(int, char **) {
  userfs::bootstrap::run();
}
