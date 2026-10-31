module;
#include "kernel/support/param.h"
#include "kernel/fs/format/layout.h"
module xv.fs:xv6_journal;
import :journal;

namespace xv::fs
{
class Xv6Journal final : public Journal
{
public:

  constexpr explicit Xv6Journal(BlockCache &cache) : cache_(cache)
  {
  }

  Xv6Journal(const Xv6Journal &) = delete;
  Xv6Journal &operator=(const Xv6Journal &) = delete;
  void initialize(int device, const superblock *layout);
  void begin() override;
  void end() override;
  void record(Buffer &buffer) override;
  void sync() override;

private:

  struct WaitProtocol;
  struct BeginFlow;
  struct EndFlow;
  struct CommitFlow;
  struct RecoverFlow;
  struct RecordFlow;
  struct SyncFlow;

  struct Header
  {
    int count = 0;
    int blocks[LOGBLOCKS]{};
  } header_;

  static_assert(sizeof(Header) < BlockSize);
  void recover();
  void install(int recovering);
  void read_header();
  void write_header();
  void write_log();
  void commit();
  BlockCache &cache_;
  xv::sync::SpinLock lock_;
  int start_ = 0;
  int outstanding_ = 0;
  int committing_ = 0;
  int device_ = 0;
  int commits_ = 0;
};

}
