module xv.drivers:pty;
import :tty;
import :n_tty;
import xv.sync;

namespace xv::drivers
{
class Pty final
{
public:
  class Output final : public TerminalOutput
  {
  public:
    constexpr explicit Output(Pty &pair) : pair_(pair) {}
    int transmit(const char *data, int count, bool nonblocking = false) override;
    void put_sync(int character) override;
    void drain() override;
    bool writable(xv::sync::PollTable *table = nullptr) override;
  private:
    Pty &pair_;
  };

  class Endpoint final : public CharacterDevice
  {
  public:
    constexpr Endpoint(Pty &pair, bool master) : pair_(pair), master_(master) {}
    const char *name() const override { return master_ ? "pty master" : "pty slave"; }
    void initialize() override {}
    int read(IoBuffer destination, int count) override;
    int write(IoBuffer source, int count) override;
    int ioctl(unsigned request, IoBuffer argument) override;
    void put_sync(int) override {}
    void close() override;
    unsigned poll(xv::sync::PollTable *table = nullptr) override;
  private:
    Pty &pair_;
    bool master_;
  };

  constexpr Pty() : output_(*this), terminal_(output_, discipline_),
                    master_(*this, true), slave_(*this, false) {}
  void initialize();
  int read_master(IoBuffer destination, int count);
  int write_master(IoBuffer source, int count);
  void close(bool master);
  void release();
  void timer_tick() { terminal_.timer_tick(); }
  Endpoint &master() { return master_; }
  Endpoint &slave() { return slave_; }
  Pty *next = nullptr;
  unsigned references = 2; // One per File, not per dup/fork descriptor.

private:
  static constexpr unsigned Capacity = 1024;
  Output output_;
  NttyDiscipline discipline_;
  Tty terminal_;
  Endpoint master_;
  Endpoint slave_;
  xv::sync::SpinLock lock_;
  xv::sync::SleepLock input_lock_;
  xv::sync::SleepLock reader_lock_;
  char data_[Capacity]{};
  unsigned read_ = 0;
  unsigned written_ = 0;
  bool master_open_ = true;
  bool slave_open_ = true;
};
}
