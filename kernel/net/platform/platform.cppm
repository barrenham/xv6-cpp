module xv.net:platform;
namespace xv::net::detail
{
void *allocate_page();
void free_page(void *page);
int owner();
bool interrupted();
int copy_in(void *destination, unsigned long source, unsigned count);
int copy_out(unsigned long destination, const void *source, unsigned count);
void prepare_sleep(void *channel);
void sleep();
void wake(void *channel);
}
