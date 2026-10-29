module xv.net:congestion;
import :wire;

namespace xv::net
{
class TcpCongestion final
{
public:

  void initialize(unsigned mss)
  {
    mss_ = mss;
    window_ = 2 * mss;
    threshold_ = 65535;
    duplicates_ = credit_ = 0;
    recovery_ = barrier_ = false;
  }

  unsigned window() const
  {
    return window_;
  }

  void timeout(unsigned flight, unsigned next)
  {
    threshold_ = flight / 2 > 2 * mss_ ? flight / 2 : 2 * mss_;
    window_ = mss_;
    credit_ = duplicates_ = 0;
    recovery_ = false;
    barrier_ = true;
    recover_ = next;
  }

  bool duplicate(unsigned acknowledgement, unsigned flight, unsigned next);

  bool acknowledged(unsigned bytes, unsigned acknowledgement);

  void reset_duplicates()
  {
    if (!recovery_)
    {
      duplicates_ = 0;
    }
  }

private:

  struct DuplicateFlow;
  struct AcknowledgeFlow;

  unsigned mss_ = 536, window_ = 1072, threshold_ = 65535, duplicates_ = 0, credit_ = 0,
           recover_ = 0;
  bool recovery_ = false, barrier_ = false;
};
} // namespace xv::net
