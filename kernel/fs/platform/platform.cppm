module xv.fs:platform;

import :inode;

namespace xv::fs::detail
{
void prepare_sleep(void *channel);
void sleep();
void wakeup(void *channel);
bool killed();
Inode *current_directory();
int copy_in(void *destination, bool user, unsigned long source, unsigned count);
int copy_out(bool user, unsigned long destination, const void *source, unsigned count);
void *allocate_page();
void free_page(void *page);
void move(void *destination, const void *source, unsigned count);
void fill(void *destination, int value, unsigned count);
int compare(const char *first, const char *second, unsigned count);
void copy_fixed(char *destination, const char *source, unsigned count);
void report(const char *message, int first = 0, int second = 0);
[[noreturn]] void fail(const char *message);
} // namespace xv::fs::detail
