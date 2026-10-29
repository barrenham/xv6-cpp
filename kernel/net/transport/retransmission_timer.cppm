module xv.net:retransmission_timer;
import :wire;

namespace xv::net
{
class RetransmissionTimer final
{
public:

  void reset()
  {
    smoothed_ = variation_ = 0;
    interval_ = 1000;
    sampling_ = ambiguous_ = false;
  }

  void start(unsigned long now)
  {
    deadline_ = now + interval_;
  }

  bool expired(unsigned long now) const
  {
    return now >= deadline_;
  }

  void sent(unsigned end, unsigned long now)
  {
    if (!sampling_ && !ambiguous_)
    {
      sample_end_ = end;
      started_ = now;
      sampling_ = true;
    }
  }

  void retransmit(unsigned long now)
  {
    uncertain();
    interval_ = interval_ < 30000 ? interval_ * 2 : 60000;
    start(now);
  }

  void uncertain()
  {
    sampling_ = false;
    ambiguous_ = true;
  }

  void acknowledge(unsigned acknowledgement, unsigned long now, bool outstanding);

private:

  struct AcknowledgeFlow;

  unsigned smoothed_ = 0, variation_ = 0, interval_ = 1000, sample_end_ = 0;
  unsigned long started_ = 0, deadline_ = 0;
  bool sampling_ = false, ambiguous_ = false;
};
} // namespace xv::net
