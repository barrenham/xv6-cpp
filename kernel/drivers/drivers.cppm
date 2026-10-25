export module xv.drivers;

export import :device;
export import :serial;
export import :character;
export import :block;
export import :interrupt;
export import :router;

export namespace xv::drivers
{
// QEMU virt board services own static instances; no allocation/registration at
// global startup. Serial/console are available before the page allocator.
void initialize_console();
void initialize_interrupts();
void initialize_hart(int hart);
void handle_external_interrupt(int hart);
CharacterDevice &console();
SerialPort &serial_port();
BlockDevice &block_device();
InterruptController &interrupt_controller();
} // namespace xv::drivers
