module xv.drivers:platform;

namespace xv::drivers::detail
{
void prepare_sleep(void *channel);
void sleep_process();
void wake_processes(void *channel);
bool process_killed();
void dump_processes();
unsigned long terminal_milliseconds();
int copy_in(void *destination, bool user, unsigned long source, unsigned count);
int copy_out(bool user, unsigned long destination, void *source, unsigned count);
void push_interrupts_off();
void pop_interrupts_off();
bool panicking();
bool panicked();
void io_barrier();
void *allocate_dma_page();
void zero_page(void *page);
[[noreturn]] void fail(const char *message);
} // namespace xv::drivers::detail
