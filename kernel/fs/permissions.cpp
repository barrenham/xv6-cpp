module;
#include "kernel/fs/fcntl.h"
#include "kernel/fs/stat.h"
module xv.fs;
import :platform;
namespace xv::fs::detail
{
bool allowed(Inode *node, unsigned requested, bool real)
{
  if (requested & ~7U) return false;
  if ((requested & W_OK) && node->owner().read_only() && node->kind() != T_DEVICE) return false;
  auto who = identity(real);
  if (who.uid == 0)
    return !(requested & X_OK) || node->kind() == T_DIR || (node->mode() & 0111);
  unsigned shift = 0;
  if (who.uid == node->uid()) shift = 6;
  else
  {
    bool group = who.gid == node->gid();
    for (unsigned i = 0; i < who.group_count; ++i) group |= who.groups[i] == node->gid();
    if (group) shift = 3;
  }
  return (((node->mode() >> shift) & requested) == requested);
}
bool may_remove(Inode *directory, Inode *target)
{
  if (!allowed(directory, W_OK | X_OK)) return false;
  auto who = identity();
  return !(directory->mode() & S_ISVTX) || who.uid == 0 || who.uid == directory->uid() || who.uid == target->uid();
}
}
