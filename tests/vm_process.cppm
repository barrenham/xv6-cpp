// Compile the real address-space owner without unrelated process services.
export module xv.process;
export import :address_space;

export namespace xv::process
{
using AddressSpaceSyncHook = void (*)(const AddressSpace &);
void set_address_space_sync_hook(AddressSpaceSyncHook);
}

namespace xv::process
{
namespace { AddressSpaceSyncHook sync_hook = nullptr; }
void set_address_space_sync_hook(AddressSpaceSyncHook hook)
{
  __atomic_store_n(&sync_hook, hook, __ATOMIC_RELEASE);
}
bool synchronize_address_space(const AddressSpace &space)
{
  // Hosted threads have coherent translations; the guest exercises hart fences.
  if (auto hook = __atomic_load_n(&sync_hook, __ATOMIC_ACQUIRE)) hook(space);
  return true;
}
void wake_address_space(const AddressSpace &) {}
void wait_address_space(const AddressSpace &) {}
}
