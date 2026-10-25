module xv.syscall:arguments;

namespace xv::syscall
{
void argument_integer(int index, int *result);
void argument_address(int index, unsigned long *result);
int argument_string(int index, char *destination, int capacity);
int fetch_address(unsigned long address, unsigned long *result);
int fetch_string(unsigned long address, char *destination, int capacity);
} // namespace xv::syscall
