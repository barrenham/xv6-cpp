module;
#include "kernel/support/flow.h"
#include "kernel/support/param.h"
#include "kernel/fs/format/layout.h"
#include "kernel/fs/stat.h"
#include "kernel/fs/fcntl.h"
#include "kernel/io/dirent.h"
module xv.fs:inode_backend_flow;
import :operations;
import :vfs;
import :platform;

namespace xv::fs
{
namespace flow = xv::flow;

struct BlockBackend::TransferFlow
{
  struct Context
  {
    BlockBackend &owner;
    xv::io::Buffer buffer;
    int count;
    bool writing;
    unsigned long offset;
    unsigned char *bytes;
    int done=0;
    unsigned start=0,take=0;
  };
  static bool more(Context &c) noexcept {return c.done<c.count && !detail::killed();}
  static void prepare(Context &c) noexcept
  {
    c.start=c.offset%512;c.take=512-c.start;
    if(c.take>unsigned(c.count-c.done))c.take=c.count-c.done;
  }
  static bool needs_read(Context &c) noexcept {return !c.writing || c.start || c.take!=512;}
  static void read_sector(Context &c) noexcept
  {c.owner.disk_->transfer(xv::drivers::BlockOperation::Read,c.offset/512,c.bytes,512);}
  static bool writing(Context &c) noexcept {return c.writing;}
  static bool copy_in(Context &c) noexcept
  {return detail::copy_in(c.bytes+c.start,true,c.buffer.address+c.done,c.take)>=0;}
  static bool copy_out(Context &c) noexcept
  {return detail::copy_out(true,c.buffer.address+c.done,c.bytes+c.start,c.take)>=0;}
  static void write_sector(Context &c) noexcept
  {c.owner.disk_->transfer(xv::drivers::BlockOperation::Write,c.offset/512,c.bytes,512);}
  static void advance(Context &c) noexcept
  {
    c.done+=c.take;c.offset+=c.take;
    __atomic_store_n(&c.owner.offset_,c.offset,__ATOMIC_RELEASE);
  }
  struct More;struct Prepare;struct Read;struct Direction;struct CopyIn;
  struct CopyOut;struct Write;struct Advance;
  using Graph=flow::graph<More,flow::types<
    flow::node<More,flow::noop,flow::branch<flow::condition<&more>,flow::go<Prepare>,flow::stop>>,
    flow::node<Prepare,flow::action<&prepare>,flow::branch<flow::condition<&needs_read>,flow::go<Read>,flow::go<Direction>>>,
    flow::node<Read,flow::action<&read_sector>,flow::go<Direction>>,
    flow::node<Direction,flow::noop,flow::branch<flow::condition<&writing>,flow::go<CopyIn>,flow::go<CopyOut>>>,
    flow::node<CopyIn,flow::noop,flow::branch<flow::condition<&copy_in>,flow::go<Write>,flow::stop>>,
    flow::node<CopyOut,flow::noop,flow::branch<flow::condition<&copy_out>,flow::go<Advance>,flow::stop>>,
    flow::node<Write,flow::action<&write_sector>,flow::go<Advance>>,
    flow::node<Advance,flow::action<&advance>,flow::go<More>>>>;
};

struct InodeBackend::MutationProtocol
{
  struct Context
  {
    InodeBackend &owner;
  };

  static void clear_setid(Context &c) noexcept
  {
    auto *node = c.owner.node_;
    if (node->mode() & 06000)
    {
      node->owner().attributes(node, node->uid(), node->gid(), node->mode() & ~06000U);
    }
  }
};

struct InodeBackend::DirectoryFlow
{
  struct Context
  {
    InodeBackend &owner;xv::io::Buffer destination;int count;
    int done=0,result=-1;unsigned long next=0;unsigned length=0,size=0;
    DirectoryRecord entry{};alignas(8) unsigned char bytes[DIRENT64_MAX_RECORD]{};
  };
  static bool more(Context &c) noexcept {return c.done<c.count;}
  static void next(Context &c) noexcept
  {
    c.next=c.owner.offset_;
    if(!c.owner.snapshot_){c.result=c.owner.node_->owner().next_directory(c.owner.node_,c.next,c.entry);return;}
    dirent legacy{};
    for(;;)
    {
      c.result=c.owner.contents_.read(c.next,false,reinterpret_cast<unsigned long>(&legacy),sizeof(legacy));
      if(c.result!=sizeof(legacy)){if(c.result)c.result=-1;return;}
      c.next+=sizeof(legacy);if(!legacy.inum)continue;
      c.entry={};c.entry.inode=legacy.inum;unsigned length=0;
      while(length<DIRSIZ && legacy.name[length])++length;
      detail::move(c.entry.name,legacy.name,length);c.entry.name[length]=0;c.result=1;return;
    }
  }
  static bool success(Context &c) noexcept {return c.result>=0;}
  static bool eof(Context &c) noexcept {return c.result==0;}
  static void finish(Context &c) noexcept
  {c.owner.offset_=c.next;__atomic_store_n(&c.owner.observed_offset_,c.next,__ATOMIC_RELEASE);}
  static void measure(Context &c) noexcept
  {
    c.length=0;while(c.length<sizeof(c.entry.name) && c.entry.name[c.length])++c.length;
    c.size=(DIRENT64_NAME_OFFSET+c.length+1+7)&~7U;
  }
  static bool valid(Context &c) noexcept
  {return c.entry.inode && c.length<sizeof(c.entry.name) && c.next>c.owner.offset_;}
  static bool fits(Context &c) noexcept {return c.size<=unsigned(c.count-c.done);}
  static void serialize(Context &c) noexcept
  {
    detail::fill(c.bytes,0,sizeof(c.bytes));
    for(unsigned i=0;i<8;++i){c.bytes[i]=c.entry.inode>>(8*i);c.bytes[8+i]=c.next>>(8*i);}
    c.bytes[16]=c.size;c.bytes[17]=c.size>>8;c.bytes[18]=c.entry.type;
    detail::move(c.bytes+DIRENT64_NAME_OFFSET,c.entry.name,c.length+1);
  }
  static bool copy(Context &c) noexcept
  {return detail::copy_out(true,c.destination.address+c.done,c.bytes,c.size)>=0;}
  static void advance(Context &c) noexcept
  {finish(c);c.owner.directory_mode_=2;c.done+=c.size;}
  struct More;struct Next;struct Eof;struct Finish;struct Measure;struct Space;struct Serialize;struct Advance;
  using Graph=flow::graph<More,flow::types<
    flow::node<More,flow::noop,flow::branch<flow::condition<&more>,flow::go<Next>,flow::stop>>,
    flow::node<Next,flow::action<&next>,flow::branch<flow::condition<&success>,flow::go<Eof>,flow::stop>>,
    flow::node<Eof,flow::noop,flow::branch<flow::condition<&eof>,flow::go<Finish>,flow::go<Measure>>>,
    flow::node<Finish,flow::action<&finish>,flow::stop>,
    flow::node<Measure,flow::action<&measure>,flow::branch<flow::condition<&valid>,flow::go<Space>,flow::stop>>,
    flow::node<Space,flow::noop,flow::branch<flow::condition<&fits>,flow::go<Serialize>,flow::stop>>,
    flow::node<Serialize,flow::action<&serialize>,flow::branch<flow::condition<&copy>,flow::go<Advance>,flow::stop>>,
    flow::node<Advance,flow::action<&advance>,flow::go<More>>>>;
};

struct InodeBackend::ReadFlow
{
  struct Context
  {
    InodeBackend &owner;
    xv::io::Buffer destination;
    int count;
    int result = -1;
  };

  static bool snapshot(Context &c) noexcept
  {
    return c.owner.snapshot_;
  }

  static void read_snapshot(Context &c) noexcept
  {
    c.result =
        c.owner.contents_.read(c.owner.offset_, true, c.destination.address, c.count);
  }

  static void read_file(Context &c) noexcept
  {
    c.result = filesystem().read(c.owner.node_, 1, c.destination.address,
                                 c.owner.offset_, c.count);
  }

  static void advance(Context &c) noexcept
  {
    if (c.result > 0)
    {
      c.owner.offset_ += c.result;
      __atomic_store_n(&c.owner.observed_offset_,c.owner.offset_,__ATOMIC_RELEASE);
    }
  }
  struct Snapshot;
  struct SnapshotRead;
  struct FileRead;
  struct Advance;
  using Graph = flow::graph<
      Snapshot,
      flow::types<
          flow::node<Snapshot, flow::noop,
                     flow::branch<flow::condition<&snapshot>, flow::go<SnapshotRead>,
                                  flow::go<FileRead>>>,
          flow::node<SnapshotRead, flow::action<&read_snapshot>, flow::go<Advance>>,
          flow::node<FileRead, flow::action<&read_file>, flow::go<Advance>>,
          flow::node<Advance, flow::action<&advance>, flow::stop>>>;
};

struct InodeBackend::WriteFlow
{
  struct Context : MutationProtocol::Context
  {
    xv::io::Buffer source;
    int count;
    int done = 0, part = 0, result = 0;
  };

  static bool more(Context &c) noexcept
  {
    return c.done < c.count;
  }

  static bool killed(Context &) noexcept
  {
    return detail::killed();
  }

  static void prepare(Context &c) noexcept
  {
    constexpr int maximum = ((MAXOPBLOCKS - 1 - 1 - 2) / 2) * BSIZE;
    c.part = c.count - c.done;
    if (c.part > maximum)
    {
      c.part = maximum;
    }
  }

  static void write_part(Context &c) noexcept
  {
    Transaction transaction(c.owner.node_->owner().persistent());
    LockedInode guard(c.owner.node_->owner(), *c.owner.node_);
    MutationProtocol::clear_setid(c);
    if (c.source.append)
    {
      c.owner.offset_ = c.owner.node_->size_bytes();
    }
    c.result = filesystem().write(c.owner.node_, 1, c.source.address + c.done,
                                  c.owner.offset_, c.part);
    if (c.result > 0)
    {
      c.owner.offset_ += c.result;
    }
    __atomic_store_n(&c.owner.observed_offset_,c.owner.offset_,__ATOMIC_RELEASE);
  }

  static void progress(Context &c) noexcept
  {
    if (c.result > 0)
    {
      c.done += c.result;
    }
  }

  static bool full(Context &c) noexcept
  {
    return c.result == c.part;
  }
  struct More;
  struct Interrupted;
  struct Prepare;
  struct Write;
  struct Progress;
  using Graph = flow::graph<
      More,
      flow::types<flow::node<More, flow::noop,
                             flow::branch<flow::condition<&more>, flow::go<Interrupted>,
                                          flow::stop>>,
                  flow::node<Interrupted, flow::noop,
                             flow::branch<flow::condition<&killed>, flow::stop,
                                          flow::go<Prepare>>>,
                  flow::node<Prepare, flow::action<&prepare>, flow::go<Write>>,
                  flow::node<Write, flow::action<&write_part>, flow::go<Progress>>,
                  flow::node<Progress, flow::action<&progress>,
                             flow::branch<flow::condition<&full>, flow::go<More>,
                                          flow::stop>>>>;
};

struct InodeBackend::SeekFlow
{
  struct Context
  {
    InodeBackend &owner;
    long offset;
    int origin;
    long base = 0, result = -1;
  };

  static bool valid(Context &c) noexcept
  {
    return c.owner.node_->kind() == T_FILE && c.origin >= SEEK_SET &&
           c.origin <= SEEK_END;
  }

  static void select_base(Context &c) noexcept
  {
    c.base = c.origin == SEEK_SET   ? 0
             : c.origin == SEEK_CUR ? c.owner.offset_
             : c.origin == SEEK_END ? (c.owner.snapshot_ ? c.owner.contents_.size()
                                                         : c.owner.node_->size_bytes())
                                    : -1;
  }

  static bool in_range(Context &c) noexcept
  {
    return !(c.base < 0 || c.offset < -c.base || c.offset > 0x7fffffffL - c.base);
  }

  static void commit(Context &c) noexcept
  {
    c.owner.offset_ = c.base + c.offset;
    __atomic_store_n(&c.owner.observed_offset_,c.owner.offset_,__ATOMIC_RELEASE);
    c.result = c.owner.offset_;
  }
  struct Validate;
  struct Base;
  struct Commit;
  using Graph = flow::graph<
      Validate,
      flow::types<
          flow::node<Validate, flow::noop,
                     flow::branch<flow::condition<&valid>, flow::go<Base>, flow::stop>>,
          flow::node<
              Base, flow::action<&select_base>,
              flow::branch<flow::condition<&in_range>, flow::go<Commit>, flow::stop>>,
          flow::node<Commit, flow::action<&commit>, flow::stop>>>;
};

struct InodeBackend::ResizeFlow
{
  struct Context : MutationProtocol::Context
  {
    unsigned size;
    int result = -1;
  };

  static bool writable(Context &c) noexcept
  {
    return !c.owner.snapshot_ && !c.owner.node_->owner().read_only();
  }

  static void resize(Context &c) noexcept
  {
    c.result = c.owner.node_->owner().resize(c.owner.node_, c.size);
  }
  struct Writable;
  struct ClearSetid;
  struct Resize;
  using Graph = flow::graph<
      Writable,
      flow::types<flow::node<Writable, flow::noop,
                             flow::branch<flow::condition<&writable>,
                                          flow::go<ClearSetid>, flow::stop>>,
                  flow::node<ClearSetid, flow::action<&MutationProtocol::clear_setid>,
                             flow::go<Resize>>,
                  flow::node<Resize, flow::action<&resize>, flow::stop>>>;
};

struct InodeBackend::SyncFlow
{
  struct Context
  {
    InodeBackend &owner;
    int result = -1;
  };

  static bool valid(Context &c) noexcept
  {
    return c.owner.node_->kind() == T_FILE || c.owner.node_->kind() == T_DIR;
  }

  static bool persistent(Context &c) noexcept
  {
    return c.owner.node_->owner().persistent();
  }

  static void primary(Context &) noexcept
  {
    journal().sync();
  }

  static bool has_secondary(Context &) noexcept
  {
    return Transaction::secondary != nullptr;
  }

  static void secondary(Context &) noexcept
  {
    Transaction::secondary->sync();
  }

  static void flush(Context &c) noexcept
  {
    c.owner.node_->owner().flush();
  }

  static void success(Context &c) noexcept
  {
    c.result = 0;
  }
  struct Validate;
  struct Persistent;
  struct Primary;
  struct Secondary;
  struct Flush;
  struct Success;
  using Graph = flow::graph<
      Validate,
      flow::types<flow::node<Validate, flow::noop,
                             flow::branch<flow::condition<&valid>, flow::go<Persistent>,
                                          flow::stop>>,
                  flow::node<Persistent, flow::noop,
                             flow::branch<flow::condition<&persistent>,
                                          flow::go<Primary>, flow::go<Success>>>,
                  flow::node<Primary, flow::action<&primary>,
                             flow::branch<flow::condition<&has_secondary>,
                                          flow::go<Secondary>, flow::go<Flush>>>,
                  flow::node<Secondary, flow::action<&secondary>, flow::go<Flush>>,
                  flow::node<Flush, flow::action<&flush>, flow::go<Success>>,
                  flow::node<Success, flow::action<&success>, flow::stop>>>;
};

} // namespace xv::fs
