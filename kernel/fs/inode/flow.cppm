module;
#include "kernel/support/flow.h"
#include "kernel/support/param.h"
#include "kernel/fs/format/layout.h"
#include "kernel/fs/stat.h"
module xv.fs:xv6_inode_flow;
import :xv6;
import :platform;

namespace xv::fs
{
namespace flow = xv::flow;

struct Xv6FileSystem::InitializeFlow
{
  struct Context
  {
    Xv6FileSystem &owner;
    int device;
  };

  static void superblock(Context &c) noexcept
  {
    c.owner.device_ = c.device;
    c.owner.read_super(c.device, &c.owner.layout_);
  }

  static bool valid(Context &c) noexcept
  {
    return c.owner.layout_.magic == FSMAGIC;
  }

  static void invalid(Context &) noexcept
  {
    detail::fail("invalid file system");
  }

  static void journal(Context &c) noexcept
  {
    c.owner.journal_.initialize(c.device, &c.owner.layout_);
  }

  static void reclaim(Context &c) noexcept
  {
    c.owner.reclaim(c.device);
  }
  struct Super;
  struct Invalid;
  struct Journal;
  struct Reclaim;
  using Graph = flow::graph<
      Super, flow::types<flow::node<Super, flow::action<&superblock>,
                                    flow::branch<flow::condition<&valid>,
                                                 flow::go<Journal>, flow::go<Invalid>>>,
                         flow::node<Invalid, flow::action<&invalid>, flow::stop>,
                         flow::node<Journal, flow::action<&journal>, flow::go<Reclaim>>,
                         flow::node<Reclaim, flow::action<&reclaim>, flow::stop>>>;
};

struct Xv6FileSystem::AllocateBlockFlow
{
  struct Context
  {
    Xv6FileSystem &owner;
    unsigned device;
    BlockHandle &block;
    int base = 0, bit = 0, mask = 0;
    bool found = false;
    unsigned result = 0;
  };

  static bool remaining(Context &c) noexcept
  {
    return c.base < c.owner.layout_.size;
  }

  static void bitmap(Context &c) noexcept
  {
    c.block = c.owner.cache_.read(c.device, BBLOCK(c.base, c.owner.layout_));
  }

  static void find(Context &c) noexcept
  {
    c.found = false;
    for (c.bit = 0; c.bit < BPB && c.base + c.bit < c.owner.layout_.size; ++c.bit)
    {
      c.mask = 1 << (c.bit % 8);
      if ((c.block->bytes()[c.bit / 8] & c.mask) == 0)
      {
        c.found = true;
        break;
      }
    }
  }

  static bool found(Context &c) noexcept
  {
    return c.found;
  }

  static void reserve(Context &c) noexcept
  {
    c.block->bytes()[c.bit / 8] |= c.mask;
    c.owner.journal_.record(*c.block);
  }

  static void release(Context &c) noexcept
  {
    c.block.reset();
  }

  static void zero(Context &c) noexcept
  {
    c.owner.zero_block(c.device, c.base + c.bit);
    c.result = c.base + c.bit;
  }

  static void advance(Context &c) noexcept
  {
    c.base += BPB;
  }

  static void exhausted(Context &) noexcept
  {
    detail::report("balloc: out of blocks\n");
  }
  struct Remaining;
  struct Bitmap;
  struct Find;
  struct Reserve;
  struct ReleaseSelected;
  struct Zero;
  struct NextBitmap;
  struct Advance;
  struct Exhausted;
  using Graph = flow::graph<
      Remaining,
      flow::types<
          flow::node<Remaining, flow::noop,
                     flow::branch<flow::condition<&remaining>, flow::go<Bitmap>,
                                  flow::go<Exhausted>>>,
          flow::node<Bitmap, flow::action<&bitmap>, flow::go<Find>>,
          flow::node<Find, flow::action<&find>,
                     flow::branch<flow::condition<&found>, flow::go<Reserve>,
                                  flow::go<NextBitmap>>>,
          flow::node<Reserve, flow::action<&reserve>, flow::go<ReleaseSelected>>,
          flow::node<ReleaseSelected, flow::action<&release>, flow::go<Zero>>,
          flow::node<Zero, flow::action<&zero>, flow::stop>,
          flow::node<NextBitmap, flow::action<&release>, flow::go<Advance>>,
          flow::node<Advance, flow::action<&advance>, flow::go<Remaining>>,
          flow::node<Exhausted, flow::action<&exhausted>, flow::stop>>>;
};

struct Xv6FileSystem::AllocateFlow
{
  struct Context
  {
    Xv6FileSystem &owner;
    unsigned device;
    short type;
    BlockHandle &block;
    int number = 1;
    dinode *disk = nullptr;
    Inode *result = nullptr;
  };

  static bool remaining(Context &c) noexcept
  {
    return c.number < c.owner.layout_.ninodes;
  }

  static void read(Context &c) noexcept
  {
    c.block = c.owner.cache_.read(c.device, IBLOCK(c.number, c.owner.layout_));
    c.disk = reinterpret_cast<dinode *>(c.block->bytes()) + c.number % IPB;
  }

  static bool free(Context &c) noexcept
  {
    return c.disk->type == 0;
  }

  static void reserve(Context &c) noexcept
  {
    detail::fill(c.disk, 0, sizeof(*c.disk));
    c.disk->type = c.type;
    c.owner.journal_.record(*c.block);
  }

  static void release(Context &c) noexcept
  {
    c.block.reset();
  }

  static void get(Context &c) noexcept
  {
    c.result = c.owner.get(c.device, c.number);
  }

  static void advance(Context &c) noexcept
  {
    ++c.number;
  }

  static void exhausted(Context &) noexcept
  {
    detail::report("ialloc: no inodes\n");
  }
  struct Remaining;
  struct Read;
  struct Reserve;
  struct ReleaseSelected;
  struct Get;
  struct ReleaseNext;
  struct Advance;
  struct Exhausted;
  using Graph = flow::graph<
      Remaining,
      flow::types<
          flow::node<Remaining, flow::noop,
                     flow::branch<flow::condition<&remaining>, flow::go<Read>,
                                  flow::go<Exhausted>>>,
          flow::node<Read, flow::action<&read>,
                     flow::branch<flow::condition<&free>, flow::go<Reserve>,
                                  flow::go<ReleaseNext>>>,
          flow::node<Reserve, flow::action<&reserve>, flow::go<ReleaseSelected>>,
          flow::node<ReleaseSelected, flow::action<&release>, flow::go<Get>>,
          flow::node<Get, flow::action<&get>, flow::stop>,
          flow::node<ReleaseNext, flow::action<&release>, flow::go<Advance>>,
          flow::node<Advance, flow::action<&advance>, flow::go<Remaining>>,
          flow::node<Exhausted, flow::action<&exhausted>, flow::stop>>>;
};

struct Xv6FileSystem::GetFlow
{
  struct Context
  {
    Xv6FileSystem &owner;
    unsigned device, number;
    Inode *result = nullptr, *empty = nullptr;
  };

  static void find(Context &c) noexcept
  {
    for (auto *node = &c.owner.inodes_[0]; node < &c.owner.inodes_[NINODE]; ++node)
    {
      if (node->ref > 0 && node->dev == c.device && node->inum == c.number)
      {
        c.result = node;
        break;
      }
      if (!c.empty && node->ref == 0)
      {
        c.empty = node;
      }
    }
  }

  static bool found(Context &c) noexcept
  {
    return c.result != nullptr;
  }

  static void retain(Context &c) noexcept
  {
    ++c.result->ref;
  }

  static bool available(Context &c) noexcept
  {
    return c.empty != nullptr;
  }

  static void exhausted(Context &) noexcept
  {
    detail::fail("iget: no inodes");
  }

  static void initialize(Context &c) noexcept
  {
    c.result = c.empty;
    c.result->dev = c.device;
    c.result->inum = c.number;
    c.result->ref = 1;
    c.result->valid = 0;
  }

  static void unlock(Context &c) noexcept
  {
    c.owner.inode_lock_.release();
  }
  struct Find;
  struct Retain;
  struct Available;
  struct Initialize;
  struct Exhausted;
  struct Unlock;
  using Graph = flow::graph<
      Find,
      flow::types<flow::node<Find, flow::action<&find>,
                             flow::branch<flow::condition<&found>, flow::go<Retain>,
                                          flow::go<Available>>>,
                  flow::node<Retain, flow::action<&retain>, flow::go<Unlock>>,
                  flow::node<Available, flow::noop,
                             flow::branch<flow::condition<&available>,
                                          flow::go<Initialize>, flow::go<Exhausted>>>,
                  flow::node<Initialize, flow::action<&initialize>, flow::go<Unlock>>,
                  flow::node<Exhausted, flow::action<&exhausted>, flow::stop>,
                  flow::node<Unlock, flow::action<&unlock>, flow::stop>>>;
};

struct Xv6FileSystem::LockFlow
{
  struct Context
  {
    Xv6FileSystem &owner;
    Inode *node;
    BlockHandle &block;
  };

  static bool live(Context &c) noexcept
  {
    return c.node && c.node->ref >= 1;
  }

  static void invalid(Context &) noexcept
  {
    detail::fail("ilock");
  }

  static void lock(Context &c) noexcept
  {
    c.node->lock.acquire();
  }

  static bool cached(Context &c) noexcept
  {
    return c.node->valid != 0;
  }

  static void load(Context &c) noexcept
  {
    c.block = c.owner.cache_.read(c.node->dev, IBLOCK(c.node->inum, c.owner.layout_));
    auto *disk = reinterpret_cast<dinode *>(c.block->bytes()) + c.node->inum % IPB;
    c.node->type = disk->type;
    c.node->major = disk->major;
    c.node->minor = disk->minor;
    c.node->nlink = disk->nlink;
    c.node->size = disk->size;
    c.node->uid_ = disk->uid;
    c.node->gid_ = disk->gid;
    c.node->mode_ = disk->mode;
    detail::move(c.node->addrs, disk->addrs, sizeof(c.node->addrs));
    c.block.reset();
    c.node->valid = 1;
  }

  static bool typed(Context &c) noexcept
  {
    return c.node->type != 0;
  }

  static void missing_type(Context &) noexcept
  {
    detail::fail("ilock: no type");
  }
  struct Live;
  struct Invalid;
  struct Lock;
  struct Load;
  struct MissingType;
  using Graph = flow::graph<
      Live,
      flow::types<flow::node<Live, flow::noop,
                             flow::branch<flow::condition<&live>, flow::go<Lock>,
                                          flow::go<Invalid>>>,
                  flow::node<Invalid, flow::action<&invalid>, flow::stop>,
                  flow::node<Lock, flow::action<&lock>,
                             flow::branch<flow::condition<&cached>, flow::stop,
                                          flow::go<Load>>>,
                  flow::node<Load, flow::action<&load>,
                             flow::branch<flow::condition<&typed>, flow::stop,
                                          flow::go<MissingType>>>,
                  flow::node<MissingType, flow::action<&missing_type>, flow::stop>>>;
};

struct Xv6FileSystem::ReleaseFlow
{
  struct Context
  {
    Xv6FileSystem &owner;
    Inode *node;
    unsigned device, number;
  };

  static void lock_node(Context &c) noexcept
  {
    c.node->lock.acquire();
    c.owner.inode_lock_.release();
  }

  static void truncate(Context &c) noexcept
  {
    c.owner.truncate(c.node);
    c.node->valid = 0;
  }

  static void unlock_node(Context &c) noexcept
  {
    c.node->lock.release();
    c.owner.inode_lock_.acquire();
  }

  static void drop(Context &c) noexcept
  {
    --c.node->ref;
    c.owner.inode_lock_.release();
  }

  static void free_inode(Context &c) noexcept
  {
    c.owner.free_inode(c.device, c.number);
  }
  struct LockNode;
  struct Truncate;
  struct UnlockNode;
  struct Drop;
  struct Free;
  using Graph = flow::graph<
      LockNode,
      flow::types<flow::node<LockNode, flow::action<&lock_node>, flow::go<Truncate>>,
                  flow::node<Truncate, flow::action<&truncate>, flow::go<UnlockNode>>,
                  flow::node<UnlockNode, flow::action<&unlock_node>, flow::go<Drop>>,
                  flow::node<Drop, flow::action<&drop>, flow::go<Free>>,
                  flow::node<Free, flow::action<&free_inode>, flow::stop>>>;
};

struct Xv6FileSystem::ReclaimFlow
{
  struct Context
  {
    Xv6FileSystem &owner;
    int device;
    int number = 1;
    Inode *node = nullptr;
  };

  static bool remaining(Context &c) noexcept
  {
    return c.number < c.owner.layout_.ninodes;
  }

  static void find(Context &c) noexcept
  {
    c.node = nullptr;
    auto block = c.owner.cache_.read(c.device, IBLOCK(c.number, c.owner.layout_));
    auto *disk = reinterpret_cast<dinode *>(block->bytes()) + c.number % IPB;
    if (disk->type != 0 && disk->nlink == 0)
    {
      detail::report("ireclaim: orphaned inode %d\n", c.number);
      c.node = c.owner.get(c.device, c.number);
    }
    block.reset();
  }

  static bool found(Context &c) noexcept
  {
    return c.node != nullptr;
  }

  static void recover(Context &c) noexcept
  {
    Transaction transaction(c.owner.journal_);
    c.owner.lock(c.node);
    c.owner.unlock(c.node);
    c.owner.release(c.node);
  }

  static void advance(Context &c) noexcept
  {
    ++c.number;
  }
  struct Remaining;
  struct Find;
  struct Recover;
  struct Advance;
  using Graph = flow::graph<
      Remaining,
      flow::types<flow::node<Remaining, flow::noop,
                             flow::branch<flow::condition<&remaining>, flow::go<Find>,
                                          flow::stop>>,
                  flow::node<Find, flow::action<&find>,
                             flow::branch<flow::condition<&found>, flow::go<Recover>,
                                          flow::go<Advance>>>,
                  flow::node<Recover, flow::action<&recover>, flow::go<Advance>>,
                  flow::node<Advance, flow::action<&advance>, flow::go<Remaining>>>>;
};

struct Xv6FileSystem::MapBlockFlow
{
  struct Context
  {
    Xv6FileSystem &owner;
    Inode *node;
    unsigned index;
    bool create;
    BlockHandle &block;
    unsigned address = 0;
    unsigned *slot = nullptr, *entries = nullptr;
  };

  static bool direct(Context &c) noexcept
  {
    return c.index < NDIRECT;
  }

  static void select_direct(Context &c) noexcept
  {
    c.slot = &c.node->addrs[c.index];
    c.address = *c.slot;
  }

  static bool present(Context &c) noexcept
  {
    return c.address != 0;
  }

  static bool create(Context &c) noexcept
  {
    return c.create;
  }

  static void allocate(Context &c) noexcept
  {
    c.address = c.owner.allocate_block(c.node->dev);
  }

  static void store(Context &c) noexcept
  {
    *c.slot = c.address;
  }

  static void indirect_index(Context &c) noexcept
  {
    c.index -= NDIRECT;
  }

  static bool indirect(Context &c) noexcept
  {
    return c.index < NINDIRECT;
  }

  static void invalid(Context &) noexcept
  {
    detail::fail("bmap: out of range");
  }

  static void select_root(Context &c) noexcept
  {
    c.slot = &c.node->addrs[NDIRECT];
    c.address = *c.slot;
  }

  static void load(Context &c) noexcept
  {
    c.block = c.owner.cache_.read(c.node->dev, c.address);
    c.entries = reinterpret_cast<unsigned *>(c.block->bytes());
    c.address = c.entries[c.index];
  }

  static void record(Context &c) noexcept
  {
    c.entries[c.index] = c.address;
    c.owner.journal_.record(*c.block);
  }

  static void release(Context &c) noexcept
  {
    c.block.reset();
  }
  struct Direct;
  struct SelectDirect;
  struct CreateDirect;
  struct AllocateDirect;
  struct StoreDirect;
  struct IndirectIndex;
  struct Invalid;
  struct SelectRoot;
  struct CreateRoot;
  struct AllocateRoot;
  struct StoreRoot;
  struct Load;
  struct CreateData;
  struct AllocateData;
  struct Record;
  struct Release;
  using Graph = flow::graph<
      Direct,
      flow::types<
          flow::node<Direct, flow::noop,
                     flow::branch<flow::condition<&direct>, flow::go<SelectDirect>,
                                  flow::go<IndirectIndex>>>,
          flow::node<SelectDirect, flow::action<&select_direct>,
                     flow::branch<flow::condition<&present>, flow::stop,
                                  flow::go<CreateDirect>>>,
          flow::node<CreateDirect, flow::noop,
                     flow::branch<flow::condition<&create>, flow::go<AllocateDirect>,
                                  flow::stop>>,
          flow::node<AllocateDirect, flow::action<&allocate>,
                     flow::branch<flow::condition<&present>, flow::go<StoreDirect>,
                                  flow::stop>>,
          flow::node<StoreDirect, flow::action<&store>, flow::stop>,
          flow::node<IndirectIndex, flow::action<&indirect_index>,
                     flow::branch<flow::condition<&indirect>, flow::go<SelectRoot>,
                                  flow::go<Invalid>>>,
          flow::node<Invalid, flow::action<&invalid>, flow::stop>,
          flow::node<SelectRoot, flow::action<&select_root>,
                     flow::branch<flow::condition<&present>, flow::go<Load>,
                                  flow::go<CreateRoot>>>,
          flow::node<CreateRoot, flow::noop,
                     flow::branch<flow::condition<&create>, flow::go<AllocateRoot>,
                                  flow::stop>>,
          flow::node<
              AllocateRoot, flow::action<&allocate>,
              flow::branch<flow::condition<&present>, flow::go<StoreRoot>, flow::stop>>,
          flow::node<StoreRoot, flow::action<&store>, flow::go<Load>>,
          flow::node<Load, flow::action<&load>,
                     flow::branch<flow::condition<&present>, flow::go<Release>,
                                  flow::go<CreateData>>>,
          flow::node<CreateData, flow::noop,
                     flow::branch<flow::condition<&create>, flow::go<AllocateData>,
                                  flow::go<Release>>>,
          flow::node<AllocateData, flow::action<&allocate>,
                     flow::branch<flow::condition<&present>, flow::go<Record>,
                                  flow::go<Release>>>,
          flow::node<Record, flow::action<&record>, flow::go<Release>>,
          flow::node<Release, flow::action<&release>, flow::stop>>>;
};

struct Xv6FileSystem::TruncateFlow
{
  struct Context
  {
    Xv6FileSystem &owner;
    Inode *node;
  };

  static void direct(Context &c) noexcept
  {
    for (unsigned i = 0; i < NDIRECT; ++i)
    {
      if (c.node->addrs[i])
      {
        c.owner.free_block(c.node->dev, c.node->addrs[i]);
        c.node->addrs[i] = 0;
      }
    }
  }

  static bool indirect(Context &c) noexcept
  {
    return c.node->addrs[NDIRECT] != 0;
  }

  static void free_indirect(Context &c) noexcept
  {
    auto block = c.owner.cache_.read(c.node->dev, c.node->addrs[NDIRECT]);
    auto *entries = reinterpret_cast<unsigned *>(block->bytes());
    for (unsigned i = 0; i < NINDIRECT; ++i)
    {
      if (entries[i])
      {
        c.owner.free_block(c.node->dev, entries[i]);
      }
    }
    block.reset();
    c.owner.free_block(c.node->dev, c.node->addrs[NDIRECT]);
    c.node->addrs[NDIRECT] = 0;
  }

  static void publish(Context &c) noexcept
  {
    c.node->size = 0;
    c.owner.update(c.node);
  }
  struct Direct;
  struct Indirect;
  struct Publish;
  using Graph = flow::graph<
      Direct,
      flow::types<flow::node<Direct, flow::action<&direct>,
                             flow::branch<flow::condition<&indirect>,
                                          flow::go<Indirect>, flow::go<Publish>>>,
                  flow::node<Indirect, flow::action<&free_indirect>, flow::go<Publish>>,
                  flow::node<Publish, flow::action<&publish>, flow::stop>>>;
};

struct Xv6FileSystem::BlockIoProtocol
{
  static unsigned minimum(unsigned first, unsigned second) noexcept
  {
    return first < second ? first : second;
  }
};

struct Xv6FileSystem::ReadFlow : BlockIoProtocol
{
  struct Context
  {
    Xv6FileSystem &owner;
    Inode *node;
    int user;
    unsigned long destination;
    unsigned offset, count;
    BlockHandle &block;
    unsigned done = 0, part = 0, address = 0;
    int result = 0;
  };

  static bool valid(Context &c) noexcept
  {
    return c.offset <= c.node->size && c.offset + c.count >= c.offset;
  }

  static void clamp(Context &c) noexcept
  {
    if (c.offset + c.count > c.node->size)
    {
      c.count = c.node->size - c.offset;
    }
  }

  static bool more(Context &c) noexcept
  {
    return c.done < c.count;
  }

  static void prepare(Context &c) noexcept
  {
    c.address = c.owner.map_block(c.node, c.offset / BSIZE, false);
    c.part = minimum(c.count - c.done, BSIZE - c.offset % BSIZE);
  }

  static bool allocated(Context &c) noexcept
  {
    return c.address != 0;
  }

  static bool copy_zero(Context &c) noexcept
  {
    static const unsigned char zeros[BSIZE]{};
    return detail::copy_out(c.user, c.destination, zeros, c.part) >= 0;
  }

  static void read(Context &c) noexcept
  {
    c.block = c.owner.cache_.read(c.node->dev, c.address);
  }

  static bool copy_data(Context &c) noexcept
  {
    return detail::copy_out(c.user, c.destination, c.block->bytes() + c.offset % BSIZE,
                            c.part) != -1;
  }

  static void release(Context &c) noexcept
  {
    c.block.reset();
  }

  static void progress(Context &c) noexcept
  {
    c.done += c.part;
    c.offset += c.part;
    c.destination += c.part;
  }

  static void complete(Context &c) noexcept
  {
    c.result = c.done;
  }

  static void sparse_failure(Context &c) noexcept
  {
    c.result = c.done ? static_cast<int>(c.done) : -1;
  }

  static void data_failure(Context &c) noexcept
  {
    c.block.reset();
    c.result = -1;
  }
  struct Valid;
  struct Clamp;
  struct Prepare;
  struct Zero;
  struct Read;
  struct Release;
  struct Progress;
  struct Complete;
  struct SparseFailure;
  struct DataFailure;
  using Graph = flow::graph<
      Valid,
      flow::types<flow::node<Valid, flow::noop,
                             flow::branch<flow::condition<&valid>, flow::go<Clamp>,
                                          flow::stop>>,
                  flow::node<Clamp, flow::action<&clamp>,
                             flow::branch<flow::condition<&more>, flow::go<Prepare>,
                                          flow::go<Complete>>>,
                  flow::node<Prepare, flow::action<&prepare>,
                             flow::branch<flow::condition<&allocated>, flow::go<Read>,
                                          flow::go<Zero>>>,
                  flow::node<Zero, flow::noop,
                             flow::branch<flow::condition<&copy_zero>,
                                          flow::go<Progress>, flow::go<SparseFailure>>>,
                  flow::node<Read, flow::action<&read>,
                             flow::branch<flow::condition<&copy_data>,
                                          flow::go<Release>, flow::go<DataFailure>>>,
                  flow::node<Release, flow::action<&release>, flow::go<Progress>>,
                  flow::node<Progress, flow::action<&progress>,
                             flow::branch<flow::condition<&more>, flow::go<Prepare>,
                                          flow::go<Complete>>>,
                  flow::node<Complete, flow::action<&complete>, flow::stop>,
                  flow::node<SparseFailure, flow::action<&sparse_failure>, flow::stop>,
                  flow::node<DataFailure, flow::action<&data_failure>, flow::stop>>>;
};

struct Xv6FileSystem::WriteFlow : BlockIoProtocol
{
  struct Context
  {
    Xv6FileSystem &owner;
    Inode *node;
    int user;
    unsigned long source;
    unsigned offset, count;
    BlockHandle &block;
    unsigned done = 0, part = 0, address = 0;
    int result = -1;
  };

  static bool valid(Context &c) noexcept
  {
    return c.offset + c.count >= c.offset && c.offset + c.count <= MAXFILE * BSIZE;
  }

  static bool more(Context &c) noexcept
  {
    return c.done < c.count;
  }

  static void map(Context &c) noexcept
  {
    c.address = c.owner.map_block(c.node, c.offset / BSIZE);
  }

  static bool mapped(Context &c) noexcept
  {
    return c.address != 0;
  }

  static void read(Context &c) noexcept
  {
    c.block = c.owner.cache_.read(c.node->dev, c.address);
    c.part = minimum(c.count - c.done, BSIZE - c.offset % BSIZE);
  }

  static bool copy(Context &c) noexcept
  {
    return detail::copy_in(c.block->bytes() + c.offset % BSIZE, c.user, c.source,
                           c.part) != -1;
  }

  static void record_release(Context &c) noexcept
  {
    c.owner.journal_.record(*c.block);
    c.block.reset();
  }

  static void progress(Context &c) noexcept
  {
    c.done += c.part;
    c.offset += c.part;
    c.source += c.part;
  }

  static void publish(Context &c) noexcept
  {
    if (c.offset > c.node->size)
    {
      c.node->size = c.offset;
    }
    c.owner.update(c.node);
    c.result = c.done;
  }
  struct Valid;
  struct More;
  struct Map;
  struct Read;
  struct RecordSuccess;
  struct RecordFailure;
  struct Progress;
  struct Publish;
  using Graph = flow::graph<
      Valid,
      flow::types<
          flow::node<Valid, flow::noop,
                     flow::branch<flow::condition<&valid>, flow::go<More>, flow::stop>>,
          flow::node<
              More, flow::noop,
              flow::branch<flow::condition<&more>, flow::go<Map>, flow::go<Publish>>>,
          flow::node<Map, flow::action<&map>,
                     flow::branch<flow::condition<&mapped>, flow::go<Read>,
                                  flow::go<Publish>>>,
          flow::node<Read, flow::action<&read>,
                     flow::branch<flow::condition<&copy>, flow::go<RecordSuccess>,
                                  flow::go<RecordFailure>>>,
          flow::node<RecordSuccess, flow::action<&record_release>, flow::go<Progress>>,
          flow::node<RecordFailure, flow::action<&record_release>, flow::go<Publish>>,
          flow::node<
              Progress, flow::action<&progress>,
              flow::branch<flow::condition<&more>, flow::go<Map>, flow::go<Publish>>>,
          flow::node<Publish, flow::action<&publish>, flow::stop>>>;
};

struct Xv6FileSystem::ResizeFlow
{
  struct Context
  {
    Xv6FileSystem &owner;
    Inode *node;
    unsigned size;
    unsigned keep = 0;
    int result = -1;
  };

  static bool valid(Context &c) noexcept
  {
    return c.node->type == T_FILE && c.size <= MAXFILE * BSIZE;
  }

  static void direct(Context &c) noexcept
  {
    c.keep = (c.size + BSIZE - 1) / BSIZE;
    for (unsigned i = c.keep; i < NDIRECT; ++i)
    {
      if (c.node->addrs[i])
      {
        c.owner.free_block(c.node->dev, c.node->addrs[i]);
        c.node->addrs[i] = 0;
      }
    }
  }

  static bool indirect(Context &c) noexcept
  {
    return c.node->addrs[NDIRECT] != 0;
  }

  static void trim_indirect(Context &c) noexcept
  {
    auto block = c.owner.cache_.read(c.node->dev, c.node->addrs[NDIRECT]);
    auto *entries = reinterpret_cast<unsigned *>(block->bytes());
    bool any = false;
    for (unsigned i = 0; i < NINDIRECT; ++i)
    {
      if (i + NDIRECT >= c.keep && entries[i])
      {
        c.owner.free_block(c.node->dev, entries[i]);
        entries[i] = 0;
      }
      any |= entries[i] != 0;
    }
    if (any)
    {
      c.owner.journal_.record(*block);
    }
    block.reset();
    if (!any)
    {
      c.owner.free_block(c.node->dev, c.node->addrs[NDIRECT]);
      c.node->addrs[NDIRECT] = 0;
    }
  }

  static bool partial_tail(Context &c) noexcept
  {
    return c.size < c.node->size && c.size % BSIZE;
  }

  static void zero_tail(Context &c) noexcept
  {
    auto address = c.owner.map_block(c.node, c.size / BSIZE, false);
    if (address)
    {
      auto block = c.owner.cache_.read(c.node->dev, address);
      detail::fill(block->bytes() + c.size % BSIZE, 0, BSIZE - c.size % BSIZE);
      c.owner.journal_.record(*block);
    }
  }

  static void publish(Context &c) noexcept
  {
    c.node->size = c.size;
    c.owner.update(c.node);
    c.result = 0;
  }
  struct Valid;
  struct Direct;
  struct Indirect;
  struct Tail;
  struct ZeroTail;
  struct Publish;
  using Graph = flow::graph<
      Valid,
      flow::types<flow::node<Valid, flow::noop,
                             flow::branch<flow::condition<&valid>, flow::go<Direct>,
                                          flow::stop>>,
                  flow::node<Direct, flow::action<&direct>,
                             flow::branch<flow::condition<&indirect>,
                                          flow::go<Indirect>, flow::go<Tail>>>,
                  flow::node<Indirect, flow::action<&trim_indirect>, flow::go<Tail>>,
                  flow::node<Tail, flow::noop,
                             flow::branch<flow::condition<&partial_tail>,
                                          flow::go<ZeroTail>, flow::go<Publish>>>,
                  flow::node<ZeroTail, flow::action<&zero_tail>, flow::go<Publish>>,
                  flow::node<Publish, flow::action<&publish>, flow::stop>>>;
};

} // namespace xv::fs
