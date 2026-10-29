module;
#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "kernel/fs/format/layout.h"
#include "kernel/fs/stat.h"

module xv.fs;
import :xv6_inode_flow;
import :platform;
import :xv6;

namespace xv::fs
{
xv::drivers::CharacterDevice *Xv6FileSystem::open_device(Inode *node, bool nonblocking)
{
  return open_registered_device(node, nonblocking);
}

// Read the super block.
void Xv6FileSystem::read_super(int dev, superblock *layout)
{
  BlockHandle bp;

  bp = cache_.read(dev, 1);
  detail::move(layout, bp->bytes(), sizeof(*layout));
  bp.reset();
}

// Init fs
void Xv6FileSystem::initialize(int dev)
{
  InitializeFlow::Context context{*this, dev};
  (void)InitializeFlow::Graph::run<0>(context);
}

// Zero a block.
void Xv6FileSystem::zero_block(int dev, int bno)
{
  BlockHandle bp;

  bp = cache_.read(dev, bno);
  detail::fill(bp->bytes(), 0, BSIZE);
  journal_.record(*bp);
  bp.reset();
}

// Blocks.

// Allocate a zeroed disk block.
// returns 0 if out of disk space.
uint Xv6FileSystem::allocate_block(uint dev)
{
  BlockHandle block;
  AllocateBlockFlow::Context context{*this, dev, block};
  (void)AllocateBlockFlow::Graph::run<0>(context);
  return context.result;
}

// Free a disk block.
void Xv6FileSystem::free_block(int dev, uint b)
{
  BlockHandle bp;
  int bi, m;

  bp = cache_.read(dev, BBLOCK(b, layout_));
  bi = b % BPB;
  m = 1 << (bi % 8);
  if ((bp->bytes()[bi / 8] & m) == 0)
  {
    detail::fail("freeing free block");
  }
  bp->bytes()[bi / 8] &= ~m;
  journal_.record(*bp);
  bp.reset();
}

// Inodes.
//
// An inode describes a single unnamed file.
// The inode disk structure holds metadata: the file's type,
// its size, the number of links referring to it, and the
// list of blocks holding the file's content.
//
// The inodes are laid out sequentially on disk at block
// layout_.inodestart. Each inode has a number, indicating its
// position on the disk.
//
// The kernel keeps a table of in-use inodes in memory
// to provide a place for synchronizing access
// to inodes used by multiple processes. The in-memory
// inodes include book-keeping information that is
// not stored on disk: ip->ref and ip->valid.
//
// An inode and its in-memory representation go through a
// sequence of states before they can be used by the
// rest of the file system code.
//
// * Allocation: an inode is allocated if its type (on disk)
//   is non-zero. allocate() allocates, and release() frees if
//   the reference and link counts have fallen to zero.
//
// * Referencing in table: an entry in the inode table
//   is free if ip->ref is zero. Otherwise ip->ref tracks
//   the number of in-memory pointers to the entry (open
//   files and current directories). get() finds or
//   creates a table entry and increments its ref; release()
//   decrements ref.
//
// * Valid: the information (type, size, &c) in an inode
//   table entry is only correct when ip->valid is 1.
//   lock() reads the inode from
//   the disk and sets ip->valid, while release() clears
//   ip->valid if ip->ref has fallen to zero.
//
// * Locked: file system code may only examine and modify
//   the information in an inode and its content if it
//   has first locked the inode.
//
// Thus a typical sequence is:
//   ip = get(dev, inum)
//   lock(ip)
//   ... examine and modify ip->xxx ...
//   unlock(ip)
//   release(ip)
//
// lock() is separate from get() so that system calls can
// get a long-term reference to an inode (as for an open file)
// and only lock it for short periods (e.g., in read()).
// The separation also helps avoid deadlock and races during
// pathname lookup. get() increments ip->ref so that the inode
// stays in the table and pointers to it remain valid.
//
// Many internal file system functions expect the caller to
// have locked the inodes involved; this lets callers create
// multi-step atomic operations.
//
// The inode_lock_ spin-lock protects the allocation of itable
// entries. Since ip->ref indicates whether an entry is free,
// and ip->dev and ip->inum indicate which i-node an entry
// holds, one must hold inode_lock_ while using any of those fields.
//
// An ip->lock sleep-lock protects all ip-> fields other than ref,
// dev, and inum.  One must hold ip->lock in order to
// read or write that inode's ip->valid, ip->size, ip->type, &c.

void Xv6FileSystem::initialize_inodes()
{
  int i = 0;

  inode_lock_.init("itable");
  for (i = 0; i < NINODE; i++)
  {
    inodes_[i].owner_ = this;
    inodes_[i].lock.init("inode");
  }
}

// Allocate an inode on device dev.
// Mark it as allocated by  giving it type type.
// Returns an unlocked but allocated and referenced inode,
// or NULL if there is no free inode.
Inode *Xv6FileSystem::allocate(uint dev, short type)
{
  BlockHandle block;
  AllocateFlow::Context context{*this, dev, type, block};
  (void)AllocateFlow::Graph::run<0>(context);
  return context.result;
}

// Copy a modified in-memory inode to disk.
// Must be called after every change to an ip->xxx field
// that lives on disk.
// Caller must hold ip->lock.
void Xv6FileSystem::update(Inode *ip)
{
  BlockHandle bp;
  struct dinode *dip;

  bp = cache_.read(ip->dev, IBLOCK(ip->inum, layout_));
  dip = (struct dinode *)bp->bytes() + ip->inum % IPB;
  dip->type = ip->type;
  dip->major = ip->major;
  dip->minor = ip->minor;
  dip->nlink = ip->nlink;
  dip->size = ip->size;
  dip->uid = ip->uid_;
  dip->gid = ip->gid_;
  dip->mode = ip->mode_;
  detail::move(dip->addrs, ip->addrs, sizeof(ip->addrs));
  journal_.record(*bp);
  bp.reset();
}

// Find the inode with number inum on device dev
// and return the in-memory copy. Does not lock
// the inode and does not read it from disk.
Inode *Xv6FileSystem::get(uint dev, uint inum)
{
  inode_lock_.acquire();
  GetFlow::Context context{*this, dev, inum};
  (void)GetFlow::Graph::run<0>(context);
  return context.result;
}

// Increment reference count for ip.
// Returns ip to enable ip = retain(ip1) idiom.
Inode *Xv6FileSystem::retain(Inode *ip)
{
  inode_lock_.acquire();
  ip->ref++;
  inode_lock_.release();
  return ip;
}

// Lock the given inode.
// Reads the inode from disk if necessary.
void Xv6FileSystem::lock(Inode *ip)
{
  BlockHandle block;
  LockFlow::Context context{*this, ip, block};
  (void)LockFlow::Graph::run<0>(context);
}

// Unlock the given inode.
void Xv6FileSystem::unlock(Inode *ip)
{
  if (ip == 0 || !ip->lock.holding() || ip->ref < 1)
  {
    detail::fail("iunlock");
  }

  ip->lock.release();
}

// Mark the on-disk inode free.
void Xv6FileSystem::free_inode(uint dev, uint inum)
{
  BlockHandle bp = cache_.read(dev, IBLOCK(inum, layout_));
  struct dinode *dip = (struct dinode *)bp->bytes() + inum % IPB;
  dip->type = 0;
  journal_.record(*bp);
  bp.reset();
}

// Drop a reference to an in-memory inode.
// If that was the last reference, the inode table entry can
// be recycled.
// If that was the last reference and the inode has no links
// to it, free the inode (and its content) on disk.
// All calls to release() must be inside a transaction in
// case it has to free the inode.
void Xv6FileSystem::release(Inode *ip)
{
  inode_lock_.acquire();
  // Keep the common reference decrement within the original short critical section.
  // Only unlinked last-reference destruction needs the multi-stage flow.
  if (ip->ref != 1 || !ip->valid || ip->nlink != 0)
  {
    --ip->ref;
    inode_lock_.release();
    return;
  }
  ReleaseFlow::Context context{*this, ip, ip->dev, ip->inum};
  (void)ReleaseFlow::Graph::run<0>(context);
}

// Common idiom: unlock, then put.
void Xv6FileSystem::unlock_release(Inode *ip)
{
  unlock(ip);
  release(ip);
}

void Xv6FileSystem::reclaim(int dev)
{
  ReclaimFlow::Context context{*this, dev};
  (void)ReclaimFlow::Graph::run<0>(context);
}

// Inode content
//
// The content (data) associated with each inode is stored
// in blocks on the disk. The first NDIRECT block numbers
// are listed in ip->addrs[].  The next NINDIRECT blocks are
// listed in block ip->addrs[NDIRECT].

// Return the disk block address of the nth block in inode ip.
// If there is no such block, bmap allocates one.
// returns 0 if out of disk space.
uint Xv6FileSystem::map_block(Inode *ip, uint bn, bool create)
{
  BlockHandle block;
  MapBlockFlow::Context context{*this, ip, bn, create, block};
  (void)MapBlockFlow::Graph::run<0>(context);
  return context.address;
}

// Truncate inode (discard contents).
// Caller must hold ip->lock.
void Xv6FileSystem::truncate(Inode *ip)
{
  TruncateFlow::Context context{*this, ip};
  (void)TruncateFlow::Graph::run<0>(context);
}

// Copy stat information from inode.
// Caller must hold ip->lock.
void Xv6FileSystem::stat(Inode *ip, Stat *st)
{
  st->dev = ip->dev;
  st->ino = ip->inum;
  st->type = ip->type;
  st->nlink = ip->nlink;
  st->size = ip->size;
  ip->security_stat(st);
}

// Read data from inode.
// Caller must hold ip->lock.
// If user_dst==1, then dst is a user virtual address;
// otherwise, dst is a kernel address.
int Xv6FileSystem::read(Inode *ip, int user_dst, uint64 dst, uint off, uint n)
{
  BlockHandle block;
  ReadFlow::Context context{*this, ip, user_dst, dst, off, n, block};
  (void)ReadFlow::Graph::run<0>(context);
  return context.result;
}

// Write data to inode.
// Caller must hold ip->lock.
// If user_src==1, then src is a user virtual address;
// otherwise, src is a kernel address.
// Returns the number of bytes successfully written.
// If the return value is less than the requested n,
// there was an error of some kind.
int Xv6FileSystem::write(Inode *ip, int user_src, uint64 src, uint off, uint n)
{
  BlockHandle block;
  WriteFlow::Context context{*this, ip, user_src, src, off, n, block};
  (void)WriteFlow::Graph::run<0>(context);
  return context.result;
}

int Xv6FileSystem::resize(Inode *node, unsigned size)
{
  ResizeFlow::Context context{*this, node, size};
  (void)ResizeFlow::Graph::run<0>(context);
  return context.result;
}

bool Xv6FileSystem::busy()
{
  xv::sync::ScopedLock guard(inode_lock_);
  unsigned references = 0;
  for (auto &node : inodes_)
  {
    references += node.ref;
  }
  return references != 1;
}

} // namespace xv::fs
