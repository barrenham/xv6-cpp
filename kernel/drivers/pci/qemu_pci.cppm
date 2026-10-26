module xv.drivers:pci;
import :mmio;
namespace xv::drivers::detail
{
bool configure_e1000(unsigned long registers);
}
