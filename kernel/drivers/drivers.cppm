export module xv.drivers;

export import :device;
export import :serial;
export import :character;
export import :block;
export import :network;
export import :interrupt;
export import :router;
export import :ldisc;

export namespace xv::drivers
{
// QEMU virt board services own static instances; no allocation/registration at
// global startup. Serial/console are available before the page allocator.
void initialize_console();
void initialize_interrupts();
void initialize_hart(int hart);
void handle_external_interrupt(int hart);
void terminal_timer_tick();
void initialize_ptys();
void pty_timer_tick();
bool create_pty(CharacterDevice **master, CharacterDevice **slave);
CharacterDevice &console();
SerialPort &serial_port();
BlockDevice &block_device();
InterruptController &interrupt_controller();
} // namespace xv::drivers
