module xv.sync:platform;

namespace xv::sync::detail
{
void push_interrupts_off();
void pop_interrupts_off();
void *current_cpu();
int current_pid();
void prepare_sleep(void *channel);
void prepare_interruptible_sleep(void *channel);
bool interrupted();
void sleep_process();
void wake_processes(void *channel);
[[noreturn]] void fail(const char *message);
} // namespace xv::sync::detail
