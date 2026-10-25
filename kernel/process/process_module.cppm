export module xv.process;

export import :registers;
export import :resources;
export import :address_space;
export import :process;
export import :scheduler;
export import :executable;

export namespace xv::process
{
void initialize();
void create_initial_process();
int fork_process();
[[noreturn]] void exit_process(int status);
int wait(unsigned long status_address);
int wait_for_child(int pid, unsigned long status_address, int options);
int kill(int pid);
int send_signal(int target, int signal);
void deliver_signals();
int set_signal_disposition(int signal, int action);
void post_terminal_interrupt(void *terminal, int signal);
void service_signal_notifications();
int claim_terminal(void *terminal);
int terminal_foreground(void *terminal);
int set_terminal_foreground(void *terminal, int group);
bool may_read_terminal(void *terminal);
int get_process_group();
int set_process_group(int pid, int group);
int get_session(int pid);
int create_session();
void prepare_sleep(void *channel);
void prepare_interruptible_sleep(void *channel);
void sleep();
void wakeup(void *channel);
void dump();
int copy_in(void *destination, bool user, unsigned long source, unsigned long count);
int copy_out(bool user, unsigned long destination, const void *source,
             unsigned long count);
} // namespace xv::process
