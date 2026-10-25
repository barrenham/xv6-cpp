export module xv.boot;

export namespace xv::boot
{
// Called on each hart in machine mode after assembly establishes its stack.
[[noreturn]] void enter(unsigned long hart_id, unsigned long device_tree);

// Supervisor-mode startup; publishes shared state before starting schedulers.
[[noreturn]] void run();
} // namespace xv::boot
