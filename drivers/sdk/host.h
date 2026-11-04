#pragma once
#include "kernel/io/kmodule_v2.h"

extern "C"
{
  const xv_kmod_host_v2 *xv_driver_host();
  void xv_driver_set_host(const xv_kmod_host_v2 *);
}
