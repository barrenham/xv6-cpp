#include "kernel/support/types.h"
#include "user/runtime/user.h"
#include "memory.h"

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
