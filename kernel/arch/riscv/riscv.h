#pragma once

// User-program compatibility header. Kernel C++ code imports xv.arch.
#include "kernel/arch/riscv/register_constants.h"
#ifndef __ASSEMBLER__
#include "kernel/arch/riscv/register_operations.inc"
#endif
