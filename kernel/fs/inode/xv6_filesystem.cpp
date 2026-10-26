module;
#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "kernel/fs/format/layout.h"
#include "kernel/fs/stat.h"

module xv.fs;
import :platform;
import :xv6;

namespace xv::fs
{
xv::drivers::CharacterDevice *Xv6FileSystem::open_device(Inode *node, bool nonblocking)
{ return open_registered_device(node, nonblocking); }

static unsigned min(unsigned first, unsigned second)
{
  return first < second ? first : second;
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
  device_ = dev;
  read_super(dev, &layout_);
  if (layout_.magic != FSMAGIC)
  {
    detail::fail("invalid file system");
  }
  journal_.initialize(dev, &layout_);
  reclaim(dev);
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
  int b, bi, m;
  BlockHandle bp;

  bp = {};
  for (b = 0; b < layout_.size; b += BPB)
  {
    bp = cache_.read(dev, BBLOCK(b, layout_));
    for (bi = 0; bi < BPB && b + bi < layout_.size; bi++)
    {
      m = 1 << (bi % 8);
      if ((bp->bytes()[bi / 8] & m) == 0)
      {                           // Is block free?
        bp->bytes()[bi / 8] |= m; // Mark block in use.
        journal_.record(*bp);
        bp.reset();
        zero_block(dev, b + bi);
        return b + bi;
      }
    }
    bp.reset();
  }
  detail::report("balloc: out of blocks\n");
  return 0;
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
  int inum;
  BlockHandle bp;
  struct dinode *dip;

  for (inum = 1; inum < layout_.ninodes; inum++)
  {
    bp = cache_.read(dev, IBLOCK(inum, layout_));
    dip = (struct dinode *)bp->bytes() + inum % IPB;
    if (dip->type == 0)
    { // a free inode
      detail::fill(dip, 0, sizeof(*dip));
      dip->type = type;
      journal_.record(*bp); // mark it allocated on the disk
      bp.reset();
      return get(dev, inum);
    }
    bp.reset();
  }
  detail::report("ialloc: no inodes\n");
  return 0;
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
  detail::move(dip->addrs, ip->addrs, sizeof(ip->addrs));
  journal_.record(*bp);
  bp.reset();
}

// Find the inode with number inum on device dev
// and return the in-memory copy. Does not lock
// the inode and does not read it from disk.
Inode *Xv6FileSystem::get(uint dev, uint inum)
{
  Inode *ip, *empty;

  inode_lock_.acquire();

  // Is the inode already in the table?
  empty = 0;
  for (ip = &inodes_[0]; ip < &inodes_[NINODE]; ip++)
  {
    if (ip->ref > 0 && ip->dev == dev && ip->inum == inum)
    {
      ip->ref++;
      inode_lock_.release();
      return ip;
    }
    if (empty == 0 && ip->ref == 0) // Remember empty slot.
    {
      empty = ip;
    }
  }

  // Recycle an inode entry.
  if (empty == 0)
  {
    detail::fail("iget: no inodes");
  }

  ip = empty;
  ip->dev = dev;
  ip->inum = inum;
  ip->ref = 1;
  ip->valid = 0;
  inode_lock_.release();

  return ip;
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
  BlockHandle bp;
  struct dinode *dip;

  if (ip == 0 || ip->ref < 1)
  {
    detail::fail("ilock");
  }

  ip->lock.acquire();

  if (ip->valid == 0)
  {
    bp = cache_.read(ip->dev, IBLOCK(ip->inum, layout_));
    dip = (struct dinode *)bp->bytes() + ip->inum % IPB;
    ip->type = dip->type;
    ip->major = dip->major;
    ip->minor = dip->minor;
    ip->nlink = dip->nlink;
    ip->size = dip->size;
    detail::move(ip->addrs, dip->addrs, sizeof(ip->addrs));
    bp.reset();
    ip->valid = 1;
    if (ip->type == 0)
    {
      detail::fail("ilock: no type");
    }
  }
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

  // Last reference of an unlinked inode?  Capture dev/inum before ref--,
  // since once ref hits 0, ip may be recycled by a concurrent get()
  // for a different inum.
  int last = (ip->ref == 1 && ip->valid && ip->nlink == 0);
  uint dev = ip->dev, inum = ip->inum;

  if (last)
  {
    // ip->ref == 1 means no other process can have ip locked.
    ip->lock.acquire();
    inode_lock_.release();

    truncate(ip); // free the data blocks (type stays nonzero on disk)
    ip->valid = 0;

    ip->lock.release();

    inode_lock_.acquire();
  }

  ip->ref--;
  inode_lock_.release();

  if (last)
  {
    free_inode(dev, inum); // now clear type on disk: inum becomes allocatable
  }
}

// Common idiom: unlock, then put.
void Xv6FileSystem::unlock_release(Inode *ip)
{
  unlock(ip);
  release(ip);
}

void Xv6FileSystem::reclaim(int dev)
{
  for (int inum = 1; inum < layout_.ninodes; inum++)
  {
    Inode *ip = 0;
    BlockHandle bp = cache_.read(dev, IBLOCK(inum, layout_));
    struct dinode *dip = (struct dinode *)bp->bytes() + inum % IPB;
    if (dip->type != 0 && dip->nlink == 0)
    { // is an orphaned inode
      detail::report("ireclaim: orphaned inode %d\n", inum);
      ip = get(dev, inum);
    }
    bp.reset();
    if (ip)
    {
      Transaction transaction(journal_);
      lock(ip);
      unlock(ip);
      release(ip);
    }
  }
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
uint Xv6FileSystem::map_block(Inode *ip, uint bn)
{
  uint addr, *a;
  BlockHandle bp;

  if (bn < NDIRECT)
  {
    if ((addr = ip->addrs[bn]) == 0)
    {
      addr = allocate_block(ip->dev);
      if (addr == 0)
      {
        return 0;
      }
      ip->addrs[bn] = addr;
    }
    return addr;
  }
  bn -= NDIRECT;

  if (bn < NINDIRECT)
  {
    // Load indirect block, allocating if necessary.
    if ((addr = ip->addrs[NDIRECT]) == 0)
    {
      addr = allocate_block(ip->dev);
      if (addr == 0)
      {
        return 0;
      }
      ip->addrs[NDIRECT] = addr;
    }
    bp = cache_.read(ip->dev, addr);
    a = (uint *)bp->bytes();
    if ((addr = a[bn]) == 0)
    {
      addr = allocate_block(ip->dev);
      if (addr)
      {
        a[bn] = addr;
        journal_.record(*bp);
      }
    }
    bp.reset();
    return addr;
  }

  detail::fail("bmap: out of range");
}

// Truncate inode (discard contents).
// Caller must hold ip->lock.
void Xv6FileSystem::truncate(Inode *ip)
{
  int i, j;
  BlockHandle bp;
  uint *a;

  for (i = 0; i < NDIRECT; i++)
  {
    if (ip->addrs[i])
    {
      free_block(ip->dev, ip->addrs[i]);
      ip->addrs[i] = 0;
    }
  }

  if (ip->addrs[NDIRECT])
  {
    bp = cache_.read(ip->dev, ip->addrs[NDIRECT]);
    a = (uint *)bp->bytes();
    for (j = 0; j < NINDIRECT; j++)
    {
      if (a[j])
      {
        free_block(ip->dev, a[j]);
      }
    }
    bp.reset();
    free_block(ip->dev, ip->addrs[NDIRECT]);
    ip->addrs[NDIRECT] = 0;
  }

  ip->size = 0;
  update(ip);
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
}

// Read data from inode.
// Caller must hold ip->lock.
// If user_dst==1, then dst is a user virtual address;
// otherwise, dst is a kernel address.
int Xv6FileSystem::read(Inode *ip, int user_dst, uint64 dst, uint off, uint n)
{
  uint tot, m;
  BlockHandle bp;

  if (off > ip->size || off + n < off)
  {
    return 0;
  }
  if (off + n > ip->size)
  {
    n = ip->size - off;
  }

  for (tot = 0; tot < n; tot += m, off += m, dst += m)
  {
    uint addr = map_block(ip, off / BSIZE);
    if (addr == 0)
    {
      break;
    }
    bp = cache_.read(ip->dev, addr);
    m = min(n - tot, BSIZE - off % BSIZE);
    if (detail::copy_out(user_dst, dst, bp->bytes() + (off % BSIZE), m) == -1)
    {
      bp.reset();
      tot = -1;
      break;
    }
    bp.reset();
  }
  return tot;
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
  uint tot, m;
  BlockHandle bp;

  if (off > ip->size || off + n < off)
  {
    return -1;
  }
  if (off + n > MAXFILE * BSIZE)
  {
    return -1;
  }

  for (tot = 0; tot < n; tot += m, off += m, src += m)
  {
    uint addr = map_block(ip, off / BSIZE);
    if (addr == 0)
    {
      break;
    }
    bp = cache_.read(ip->dev, addr);
    m = min(n - tot, BSIZE - off % BSIZE);
    if (detail::copy_in(bp->bytes() + (off % BSIZE), user_src, src, m) == -1)
    {
      // Might have partially updated the block, so we need to log it.
      journal_.record(*bp);
      bp.reset();
      break;
    }
    journal_.record(*bp);
    bp.reset();
  }

  if (off > ip->size)
  {
    ip->size = off;
  }

  // write the i-node back to disk even if the size didn't change
  // because the loop above might have called map_block() and added a new
  // block to ip->addrs[].
  update(ip);

  return tot;
}

} // namespace xv::fs
