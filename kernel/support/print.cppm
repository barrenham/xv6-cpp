export module xv.print;

export namespace xv::print
{
void initialize();
int printk(const char *format, ...) __attribute__((format(printf, 1, 2)));
[[noreturn]] void panic(const char *message);
bool panicking();
bool panicked();
} // namespace xv::print
