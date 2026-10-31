module xv.drivers:platform;

namespace xv::drivers::detail
{
unsigned effective_uid();
unsigned effective_gid();
void prepare_sleep(void *channel);
void prepare_interruptible_sleep(void *channel);
void sleep_process();
void wake_processes(void *channel);
bool process_killed();
void *pin_controlling_terminal(void *(*pin)(void *));
int claim_terminal(void *terminal);
int terminal_foreground(void *terminal);
int set_terminal_foreground(void *terminal, int group);
bool may_read_terminal(void *terminal);
void post_terminal_interrupt(void *terminal, bool suspend);
void dump_processes();
void detach_terminal(void *terminal);
void hangup_terminal(void *terminal);
void service_terminal_signals();
void free_terminal_page(void *page);
unsigned long terminal_milliseconds();
int copy_in(void *destination, bool user, unsigned long source, unsigned count);
int copy_out(bool user, unsigned long destination, void *source, unsigned count);
void push_interrupts_off();
void pop_interrupts_off();
bool panicking();
bool panicked();
void io_barrier();
void *allocate_dma_page();
void free_dma_page(void *page);
void zero_page(void *page);
[[noreturn]] void fail(const char *message);
} // namespace xv::drivers::detail
