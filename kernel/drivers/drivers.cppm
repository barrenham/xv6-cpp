module;
#include "kernel/io/kmodule.h"
#include "kernel/io/kmodule_v2.h"
#include "kernel/io/kmodule_v3.h"
#include "kernel/io/kmodule_v4.h"
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
// Board services expose generic proxies. Resident ELF modules own concrete
// drivers; early polling output is available before the page allocator.
void initialize_console();
unsigned long realtime_nanoseconds();
unsigned long realtime_seconds();
unsigned long boot_time_seconds();
void initialize_interrupts();
void initialize_hart(int hart);
void handle_external_interrupt(int hart);
void terminal_timer_tick();
void initialize_ptys();
void pty_timer_tick();
bool create_pty(CharacterDevice **master, CharacterDevice **slave);
inline constexpr unsigned MaxNamedPtys = 64;
CharacterDevice &ptmx();
bool retain_named_pty(unsigned number);
bool named_pty_owner(unsigned number, unsigned &uid, unsigned &gid);
void release_named_pty(unsigned number);
CharacterDevice *open_named_pty(unsigned number);
unsigned named_pty_ids(unsigned *numbers, unsigned capacity);
CharacterDevice &console();
SerialPort &serial_port();
BlockDevice &block_device();
BlockDevice *data_device();
void initialize_modules();
void initialize_boot_modules();
void start_hotplug_worker();
void register_network_hotplug(void (*callback)(unsigned, bool));
unsigned scan_device_resources();
bool device_hotplug_status(unsigned index, xv_kmod_hotplug_status &status);
bool device_removal_requested(unsigned index, bool &surprise);
bool eject_device_resource(unsigned index);
long module_control(unsigned operation, unsigned long argument, unsigned long value);
bool module_device(unsigned index, xv_kmod_device &device);
bool module_device_v2(unsigned index, xv_kmod_device_v2 &device);
bool module_device_v3(unsigned index, xv_kmod_device_v3 &device);
void module_interrupt(unsigned index);
InterruptController &interrupt_controller();
} // namespace xv::drivers
