module;
#include "kernel/support/types.h"
#include "user/runtime/user.h"
module xv.userfs.ramfs;
import xv.userfs;

namespace userfs
{
namespace
{
class HeapStorage final : public MemoryStorage
{
public:
  void *allocate(unsigned bytes) override { return malloc(bytes); }
  void release(void *memory) override { free(memory); }
};
constinit HeapStorage storage;
}
MemoryStorage &heap_storage() { return storage; }
} // namespace userfs
