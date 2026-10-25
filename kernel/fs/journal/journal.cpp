module;
#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "kernel/fs/format/layout.h"
#include "kernel/fs/stat.h"

module xv.fs;
import :platform;

namespace xv::fs
{
void Journal::initialize(int dev, const superblock *sb)
{

  lock_.init("log");
  start_ = sb->logstart;
  device_ = dev;
  recover();
}

// Copy committed blocks from log to their home location
void Journal::install(int recovering)
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
void Journal::read_header(void)
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
void Journal::write_header(void)
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

void Journal::recover(void)
{
  read_header();
  install(1); // if committed, copy from log to disk
  header_.count = 0;
  write_header(); // clear the log
}

// called at the start of each FS system call.
void Journal::begin(void)
{
  lock_.acquire();
  while (1)
  {
    if (committing_)
    {
      detail::prepare_sleep(this);
      lock_.release();
      detail::sleep();
      lock_.acquire();
    }
    else if (header_.count + (outstanding_ + 1) * MAXOPBLOCKS > LOGBLOCKS)
    {
      // this op might exhaust log space; wait for commit.
      detail::prepare_sleep(this);
      lock_.release();
      detail::sleep();
      lock_.acquire();
    }
    else
    {
      outstanding_ += 1;
      lock_.release();
      break;
    }
  }
}

// called at the end of each FS system call.
// commits if this was the last outstanding operation.
void Journal::end(void)
{
  int do_commit = 0;

  lock_.acquire();
  outstanding_ -= 1;
  if (committing_)
  {
    detail::fail("committing_");
  }
  if (outstanding_ == 0)
  {
    do_commit = 1;
    committing_ = 1;
  }
  else
  {
    // begin() may be waiting for log space,
    // and decrementing outstanding_ has decreased
    // the amount of reserved space.
    detail::wakeup(this);
  }
  lock_.release();

  if (do_commit)
  {
    // call commit w/o holding locks, since not allowed
    // to sleep with locks.
    commit();
    lock_.acquire();
    committing_ = 0;
    commits_ += 1;
    detail::wakeup(this);
    lock_.release();
  }
}

// Copy modified blocks from cache to log.
void Journal::write_log(void)
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

void Journal::commit()
{
  if (header_.count > 0)
  {
    write_log();    // Write modified blocks from cache to log
    write_header(); // Write header to disk -- the real commit
    install(0);     // Now install writes to home locations
    header_.count = 0;
    write_header(); // Erase the transaction from the log
  }
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
void Journal::record(Buffer &buffer)
{
  auto *b = &buffer;
  int i;

  lock_.acquire();
  if (header_.count >= LOGBLOCKS)
  {
    detail::fail("too big a transaction");
  }
  if (outstanding_ < 1)
  {
    detail::fail("log_write outside of trans");
  }

  for (i = 0; i < header_.count; i++)
  {
    if (header_.blocks[i] == b->block_number()) // log absorption
    {
      break;
    }
  }
  header_.blocks[i] = b->block_number();
  if (i == header_.count)
  { // Add new block to log?
    cache_.pin(*b);
    header_.count++;
  }
  lock_.release();
}

void Journal::sync(void)
{
  lock_.acquire();
  if (committing_ || outstanding_ > 0)
  {
    int n = commits_ + 1;
    while (commits_ < n)
    {
      detail::prepare_sleep(this);
      lock_.release();
      detail::sleep();
      lock_.acquire();
    }
  }
  lock_.release();
}

} // namespace xv::fs
