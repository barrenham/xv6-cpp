module;
#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "kernel/fs/format/layout.h"
#include "kernel/fs/stat.h"

module xv.fs;
import :journal_flow;
import :platform;

namespace xv::fs
{
void Xv6Journal::initialize(int dev, const superblock *sb)
{

  lock_.init("log");
  start_ = sb->logstart;
  device_ = dev;
  recover();
}

// Copy committed blocks from log to their home location
void Xv6Journal::install(int recovering)
{
  int tail;

  for (tail = 0; tail < header_.count; tail++)
  {
    if (recovering)
    {
      detail::report("recovering tail %d dst %d\n", tail, header_.blocks[tail]);
    }
    BlockHandle lbuf = cache_.read(device_, start_ + tail + 1);    // read log block
    BlockHandle dbuf = cache_.read(device_, header_.blocks[tail]); // read dst
    detail::move(dbuf->bytes(), lbuf->bytes(), BSIZE);             // copy block to dst
    cache_.write(*dbuf);                                           // write dst to disk
    if (recovering == 0)
    {
      cache_.unpin(*dbuf);
    }
    lbuf.reset();
    dbuf.reset();
  }
}

// Read the log header from disk into the in-memory log header
void Xv6Journal::read_header(void)
{
  BlockHandle buf = cache_.read(device_, start_);
  Header *lh = (Header *)(buf->bytes());
  int i;
  header_.count = lh->count;
  for (i = 0; i < header_.count; i++)
  {
    header_.blocks[i] = lh->blocks[i];
  }
  buf.reset();
}

// Write in-memory log header to disk.
// This is the true point at which the
// current transaction commits.
void Xv6Journal::write_header(void)
{
  BlockHandle buf = cache_.read(device_, start_);
  Header *hb = (Header *)(buf->bytes());
  int i;
  hb->count = header_.count;
  for (i = 0; i < header_.count; i++)
  {
    hb->blocks[i] = header_.blocks[i];
  }
  cache_.write(*buf);
  buf.reset();
}

void Xv6Journal::recover(void)
{
  RecoverFlow::Context context{*this};
  (void)RecoverFlow::Graph::run<0>(context);
}

// called at the start of each FS system call.
void Xv6Journal::begin(void)
{
  lock_.acquire();
  BeginFlow::Context context{*this};
  (void)BeginFlow::Graph::run<0>(context);
}

// called at the end of each FS system call.
// commits if this was the last outstanding operation.
void Xv6Journal::end(void)
{
  lock_.acquire();
  EndFlow::Context context{*this};
  (void)EndFlow::Graph::run<0>(context);
}

// Copy modified blocks from cache to log.
void Xv6Journal::write_log(void)
{
  int tail;

  for (tail = 0; tail < header_.count; tail++)
  {
    BlockHandle to = cache_.read(device_, start_ + tail + 1);      // log block
    BlockHandle from = cache_.read(device_, header_.blocks[tail]); // cache block
    detail::move(to->bytes(), from->bytes(), BSIZE);
    cache_.write(*to); // write the log
    from.reset();
    to.reset();
  }
}

void Xv6Journal::commit()
{
  CommitFlow::Context context{*this};
  (void)CommitFlow::Graph::run<0>(context);
}

// Caller has modified b->bytes() and is done with the buffer.
// Record the block number and pin in the cache by increasing refcnt.
// commit()/write_log() will do the disk write.
//
// record() replaces bwrite(); a typical use is:
//   bp = cache_.read(...)
//   modify bp->bytes()[]
//   record(bp)
//   brelse(bp)
void Xv6Journal::record(Buffer &buffer)
{
  lock_.acquire();
  RecordFlow::Context context{{*this}, buffer};
  (void)RecordFlow::Graph::run<0>(context);
}

void Xv6Journal::sync(void)
{
  lock_.acquire();
  SyncFlow::Context context{{*this}};
  (void)SyncFlow::Graph::run<0>(context);
}

} // namespace xv::fs
