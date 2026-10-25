export module xv.syscall;

export namespace xv::syscall
{
// Read a7, invoke a module-owned handler, and write the result to a0.
void dispatch();
} // namespace xv::syscall
