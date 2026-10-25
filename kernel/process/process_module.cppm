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
int kill(int pid);
void prepare_sleep(void *channel);
void sleep();
void wakeup(void *channel);
void dump();
int copy_in(void *destination, bool user, unsigned long source, unsigned long count);
int copy_out(bool user, unsigned long destination, const void *source,
             unsigned long count);
} // namespace xv::process
