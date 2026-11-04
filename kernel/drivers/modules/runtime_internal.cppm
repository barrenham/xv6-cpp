module;
#include "kernel/io/kmodule_v2.h"
module xv.drivers:module_runtime;
import xv.sync;
import :serial;
import :network;

namespace xv::drivers::modules
{
inline constexpr unsigned MaxModules = 32;
inline constexpr unsigned MaxDevices = 16;

class ResourcePool
{
public:

  constexpr ResourcePool() = default;
  void initialize();
  void *allocate();
  void release(void *page, bool shared);
  bool adopt(void *page);
  void detach(void *page);

private:

  struct Slot
  {
    void *address;
    Slot *next;
  };

  struct Node
  {
    Node *next;
    Slot slots[255];
  };

  static_assert(sizeof(Node) <= 4096);
  Node *nodes_ = nullptr;
  Slot *free_ = nullptr;
  unsigned count_ = 0;
  xv::sync::SpinLock lock_;
};

struct Image
{
  unsigned long base = 0;
  unsigned long bytes = 0;
  const xv_kmod_driver *driver = nullptr;
  unsigned bound = 0;
  unsigned long transfers = 0;
  unsigned long interrupts = 0;
  ResourcePool pool;
  xv_kmod_host_v2 services{};
};

struct Binding
{
  Image *image = nullptr;
  unsigned irq = 0;
  unsigned index = 0;
  xv_kmod_host host{};
  xv_kmod_block operations{};
  void *pages[32]{};
  unsigned page_count = 0;
  void *interrupt_context = nullptr;
  void (*interrupt_handler)(void *) = nullptr;
  unsigned interrupt_active = 0;
  xv::sync::SpinLock interrupt_lock;
  xv_kmod_serial serial{};
  xv_kmod_ethernet ethernet{};
  unsigned network_slot = ~0U;
};

extern Image images[MaxModules];
extern Binding bindings[MaxDevices];
extern unsigned online[MaxDevices];
bool callback(const Image &, unsigned long address);
int attach(Image &, const xv_kmod_device_v2 &);
SerialPort &serial_proxy();
unsigned network_count();
EthernetDevice &network_proxy(unsigned index);
void network_receiver(unsigned index, EthernetReceiver &receiver);
} // namespace xv::drivers::modules

extern "C" void xv_kmod_synchronize();
