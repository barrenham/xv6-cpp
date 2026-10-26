module xv.drivers:pci;
import :mmio;
namespace xv::drivers::detail
{
unsigned find_e1000(unsigned *slots, unsigned capacity);
bool configure_e1000(unsigned long registers, unsigned slot);
}
