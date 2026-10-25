#pragma once
#include "kernel/drivers/device_numbers.h"

// map major device number to device functions.
struct devsw {
  int (*read)(int, uint64, int);
  int (*write)(int, uint64, int);
};

extern "C" struct devsw devsw[];
