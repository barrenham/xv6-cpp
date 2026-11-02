#include "kernel/support/types.h"
#include "kernel/fs/stat.h"
#include "user/runtime/user.h"
#include "xv6.h"

namespace userfs
{
Xv6Backend::~Xv6Backend() { if (state_) free(state_); }
bool Xv6Backend::initialize()
{
  if (!volume_.mount()) return false;
  state_ = static_cast<State *>(malloc(volume_.inodes() * sizeof(State)));
  if (!state_) return false;
  memset(state_, 0, volume_.inodes() * sizeof(State));
  state_[ROOTINO].parent = ROOTINO;
  return true;
}
Attributes Xv6Backend::attributes(const xv6::Inode &node)
{
  const auto &value = node.value;
  return {node.number, value.size, value.uid, value.gid, value.mode,
          unsigned(value.type), unsigned(value.nlink), unsigned(value.major), unsigned(value.minor)};
}
Attributes Xv6Backend::root()
{
  xv6::Inode node;
  return state_ && volume_.read_inode(ROOTINO, node) ? attributes(node) : Attributes{};
}
bool Xv6Backend::borrow(const xv6::Inode &node)
{
  if (!state_ || failed_) return false;
  if (node.value.type == T_DIR)
  {
    unsigned current = node.number;
    for (unsigned depth = 0; current != ROOTINO; ++depth)
    {
      if (depth >= volume_.inodes()) return false;
      if (!state_[current].parent)
      {
        xv6::Inode parent;
        if (!volume_.lookup(current, "..", parent) || parent.value.type != T_DIR) return false;
        state_[current].parent = parent.number;
      }
      current = state_[current].parent;
    }
  }
  state_[node.number].borrowed = true;
  return true;
}
bool Xv6Backend::retained(unsigned number) const
{
  if (number == ROOTINO || state_[number].borrowed) return true;
  // Unlinked directory handles preserve their parent chain until final Forget.
  for (unsigned i = 2; i < volume_.inodes(); ++i)
  {
    if (!state_[i].borrowed || !state_[i].parent) continue;
    unsigned parent = i;
    for (unsigned depth = 0; parent != ROOTINO && depth < volume_.inodes(); ++depth)
    {
      parent = state_[parent].parent;
      if (parent == number) return true;
      if (!parent) break;
    }
  }
  return false;
}
bool Xv6Backend::collect()
{
  if (!state_ || failed_) return false;
  for (unsigned i = 2; i < volume_.inodes(); ++i)
  {
    if (!state_[i].orphan || retained(i)) continue;
    if (!volume_.reclaim(i)) { failed_ = true; return false; }
    state_[i] = {};
  }
  return true;
}
int Xv6Backend::lookup(unsigned parent, const char *name, Attributes &out)
{
  xv6::Inode node;
  if (!state_ || failed_ || !volume_.lookup(parent, name, node) || !borrow(node)) return -1;
  out = attributes(node); return 0;
}
int Xv6Backend::getattr(unsigned number, Attributes &out)
{
  xv6::Inode node;
  if (!state_ || failed_ || !volume_.read_inode(number, node)) return -1;
  out = attributes(node); return 0;
}
int Xv6Backend::readdir(unsigned number, unsigned long offset, Entry &out)
{
  if (!state_ || failed_) return -1;
  xv6::Entry entry;
  if (!volume_.next_entry(number, offset, entry)) return volume_.error() == xv6::Error::None ? 0 : -1;
  xv6::Inode node;
  if (!volume_.read_inode(entry.inode, node)) return -1;
  out.attributes = attributes(node); out.next = offset;
  strcpy(out.name, entry.name); return 1;
}
int Xv6Backend::read(unsigned number, unsigned long offset, void *out, unsigned count)
{
  return state_ && !failed_ ? volume_.read(number, offset, out, count) : -1;
}
int Xv6Backend::write(unsigned number, unsigned long offset, const void *in, unsigned count)
{
  return state_ && !failed_ ? volume_.write(number, offset, in, count) : -1;
}
int Xv6Backend::create(unsigned parent, const char *name, const Attributes &requested, Attributes &out)
{
  xv6::Inode node;
  if (!state_ || failed_ || !volume_.create(parent, name, requested.type, requested.mode,
      requested.uid, requested.gid, requested.major, requested.minor, node)) return -1;
  state_[node.number] = {};
  if (!borrow(node)) return -1;
  out = attributes(node); return 0;
}
int Xv6Backend::resize(unsigned number, unsigned size)
{
  return state_ && !failed_ && volume_.resize(number, size) ? 0 : -1;
}
int Xv6Backend::setattr(unsigned number, const Attributes &requested)
{
  return state_ && !failed_ && volume_.setattr(number, requested.uid, requested.gid, requested.mode) ? 0 : -1;
}
int Xv6Backend::link(unsigned parent, const char *name, unsigned target)
{
  return state_ && !failed_ && volume_.link(parent, name, target) ? 0 : -1;
}
int Xv6Backend::unlink(unsigned parent, const char *name, Attributes &out)
{
  xv6::Inode removed;
  if (!state_ || failed_ || !volume_.unlink(parent, name, removed)) return -1;
  out = attributes(removed);
  if (!removed.value.nlink) state_[removed.number].orphan = true;
  return collect() ? 0 : -1;
}
int Xv6Backend::rename(unsigned from, const char *old_name, unsigned to, const char *new_name)
{
  xv6::Inode source;
  unsigned replaced = 0;
  if (!state_ || failed_ || !volume_.lookup(from, old_name, source) ||
      !volume_.rename(from, old_name, to, new_name, replaced)) return -1;
  if (source.value.type == T_DIR) state_[source.number].parent = to;
  if (replaced)
  {
    xv6::Inode node;
    if (!volume_.read_inode(replaced, node)) return -1;
    if (!node.value.nlink) state_[replaced].orphan = true;
  }
  return collect() ? 0 : -1;
}
int Xv6Backend::sync(unsigned)
{
  return state_ && !failed_ && collect() && volume_.sync() ? 0 : -1;
}
void Xv6Backend::forget(unsigned number)
{
  if (!state_ || number >= volume_.inodes()) return;
  state_[number].borrowed = false;
  if (!collect()) failed_ = true;
}
} // namespace userfs
