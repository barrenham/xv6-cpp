#pragma once
#include "kernel/io/posix_errno.h"
#ifdef __cplusplus
extern "C" int *__errno_location(void) noexcept;
#else
int *__errno_location(void);
#endif
#ifndef errno
#define errno (*__errno_location())
#endif
