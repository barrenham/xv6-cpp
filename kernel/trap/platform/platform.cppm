module xv.trap:platform;
import :event;

namespace xv::trap::detail
{
TrapEvent snapshot();
void install_kernel_vector();
bool has_process();
bool killed();
bool interrupted();
void deliver_signals();
void request_kill();
[[noreturn]] void exit_current(int status);
void yield_current();
void save_user_pc(Word pc);
void advance_user_pc(Word bytes);
void enable_interrupts();
void dispatch_system_call();
void dispatch_external_interrupt();
bool resolve_page_fault(Word address, MemoryAccess access);
void report_user_fault(const TrapEvent &event);
[[noreturn]] void fatal(const char *message, const TrapEvent &event);
void prepare_user_return();
Word user_page_table_token();
void restore_kernel(const TrapEvent &event);
[[noreturn]] void enter_user(Word page_table_token);
int hart_id();
Word time();
void arm_timer(Word deadline);
void service_timer_devices();
void prepare_sleep(void *channel);
void prepare_interruptible_sleep(void *channel);
void sleep();
void wakeup(void *channel);
} // namespace xv::trap::detail
