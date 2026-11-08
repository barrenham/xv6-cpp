module xv.process;
import xv.memory;

namespace xv::process
{
AddressSpace::Access::Access(const AddressSpace &owner) : owner_(owner) {
  __atomic_add_fetch(&owner_.active_accesses_, 1U, __ATOMIC_SEQ_CST);
  xv::sync::push_interrupts_off();
  if (!owner_.operation_lock_.holding())
  {
    owner_.operation_lock_.acquire();
  }
  ++owner_.operation_depth_;
  xv::sync::pop_interrupts_off();
}

AddressSpace::Access::~Access() {
  bool notify = false;
  if (!--owner_.operation_depth_)
  {
    if (owner_.operation_blocked_)
    {
      owner_.operation_blocked_ = false;
      notify = !__atomic_sub_fetch(&owner_.user_blocks_, 1U, __ATOMIC_SEQ_CST);
    }
    owner_.operation_lock_.release();
  }
  __atomic_sub_fetch(&owner_.active_accesses_, 1U, __ATOMIC_SEQ_CST);
  if (notify)
  {
    wake_address_space(owner_);
  }
}

AddressSpace::Suspension::Suspension(const AddressSpace &owner, bool keep_block)
    : owner_(owner), depth_(owner.operation_depth_),
      blocked_(keep_block && owner.operation_blocked_) {
  if (!depth_ || !owner_.operation_lock_.holding())
  {
    __builtin_trap();
  }
  bool notify = false;
  if (owner_.operation_blocked_ && !keep_block)
  {
    notify = !__atomic_sub_fetch(&owner_.user_blocks_, 1U, __ATOMIC_SEQ_CST);
  }
  owner_.operation_depth_ = 0;
  owner_.operation_blocked_ = false;
  owner_.operation_lock_.release();
  if (notify)
  {
    wake_address_space(owner_);
  }
}

AddressSpace::Suspension::~Suspension() {
  owner_.operation_lock_.acquire();
  owner_.operation_depth_ = depth_;
  owner_.operation_blocked_ = blocked_;
}

void
AddressSpace::enable_concurrency() {
  Access access(*this);
  shared_execution_ = true;
}

bool
AddressSpace::operation_held() const {
  xv::sync::push_interrupts_off();
  bool held = operation_lock_.holding();
  xv::sync::pop_interrupts_off();
  return held;
}

bool
AddressSpace::user_entry_allowed() const {
  return !__atomic_load_n(&user_blocks_, __ATOMIC_SEQ_CST) &&
         !__atomic_load_n(&retiring_, __ATOMIC_ACQUIRE);
}

bool
AddressSpace::refresh_for_user() const {
  Access access(*this);
  if (!user_entry_allowed() || !table_)
  {
    return false;
  }
  refresh_file_mappings();
  return true;
}

bool
AddressSpace::invalidate_translations() const {
  if (shared_execution_ && table_ && !operation_blocked_)
  {
    // A mutation keeps user entry closed while hart quiescence waits outside
    // the page-table spin lock. Kernel copies remain able to make progress.
    __atomic_add_fetch(&user_blocks_, 1U, __ATOMIC_SEQ_CST);
    operation_blocked_ = true;
    {
      Suspension unlocked(*this, true);
      if (!synchronize_address_space(*this))
      {
        return false;
      }
    }
  }
  ++generation_;
  return true;
}

bool
AddressSpace::prepare_write(unsigned long address, unsigned long bytes) const {
  constexpr unsigned long Page = 4096, Limit = 1UL << 38;
  if (bytes > Limit || address > Limit - bytes)
  {
    return false;
  }
  for (auto at = address; at < address + bytes; at = (at & ~(Page - 1)) + Page)
  {
    auto *pte = xv::memory::walk_entry(table_, at, 0);
    if (pte && (*pte & (1UL << 8)))
    {
      return invalidate_translations();
    }
  }
  return true;
}

} // namespace xv::process
