export module xv.process:credentials;
import xv.sync;
export namespace xv::process
{
struct CredentialState
{
  unsigned uid = 0, effective_uid = 0, saved_uid = 0;
  unsigned gid = 0, effective_gid = 0, saved_gid = 0;
  unsigned groups[16]{};
  unsigned group_count = 0;
  unsigned mask = 0022;
};
class Credentials final
{
public:
  constexpr Credentials() = default;
  CredentialState snapshot() const
  { xv::sync::ScopedLock guard(lock_); return state_; }
  void inherit(const Credentials &parent)
  { auto copy = parent.snapshot(); xv::sync::ScopedLock guard(lock_); state_ = copy; }
  int set_uid(unsigned value, bool effective_only = false)
  {
    if (value > 65534) return -1;
    xv::sync::ScopedLock guard(lock_);
    if (state_.effective_uid != 0 && value != state_.uid && value != state_.saved_uid) return -1;
    if (!effective_only && state_.effective_uid == 0) state_.uid = state_.saved_uid = value;
    state_.effective_uid = value; return 0;
  }
  int set_gid(unsigned value, bool effective_only = false)
  {
    if (value > 65534) return -1;
    xv::sync::ScopedLock guard(lock_);
    if (state_.effective_uid != 0 && value != state_.gid && value != state_.saved_gid) return -1;
    if (!effective_only && state_.effective_uid == 0) state_.gid = state_.saved_gid = value;
    state_.effective_gid = value; return 0;
  }
  int set_groups(const unsigned *groups, unsigned count)
  {
    if (count > 16) return -1;
    for (unsigned i = 0; i < count; ++i) if (groups[i] > 65534) return -1;
    xv::sync::ScopedLock guard(lock_);
    if (state_.effective_uid != 0) return -1;
    state_.group_count = count;
    for (unsigned i = 0; i < count; ++i) state_.groups[i] = groups[i];
    return 0;
  }
  void apply_exec(unsigned uid, unsigned gid, unsigned mode)
  {
    xv::sync::ScopedLock guard(lock_);
    if (mode & 04000) state_.effective_uid = uid;
    if (mode & 02000) state_.effective_gid = gid;
    state_.saved_uid = state_.effective_uid; state_.saved_gid = state_.effective_gid;
  }
  unsigned set_mask(unsigned mask)
  { xv::sync::ScopedLock guard(lock_); unsigned old = state_.mask; state_.mask = mask & 0777; return old; }
  static bool may_signal(const CredentialState &sender, const CredentialState &target)
  { return sender.effective_uid == 0 || sender.uid == target.uid || sender.uid == target.saved_uid ||
           sender.effective_uid == target.uid || sender.effective_uid == target.saved_uid; }
private:
  mutable xv::sync::SpinLock lock_;
  CredentialState state_;
};
}
