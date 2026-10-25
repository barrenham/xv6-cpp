module;

#include "kernel/arch/riscv/register_constants.h"

export module xv.arch;

export namespace xv::arch
{
#include "kernel/arch/riscv/register_operations.inc"
} // namespace xv::arch
