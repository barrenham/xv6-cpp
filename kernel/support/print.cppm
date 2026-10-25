module;

#include "kernel/support/print_abi.h"

export module xv.print;

export namespace xv::print
{
void initialize();
using ::printk;
using ::panic;
} // namespace xv::print
