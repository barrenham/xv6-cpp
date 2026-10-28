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

  void acknowledge(unsigned acknowledgement, unsigned long now, bool outstanding)
  {
    if (sampling_ && !wire::before(acknowledgement, sample_end_))
    {
      unsigned sample = now - started_;
      if (!sample)
      {
        sample = 1;
      }
      if (!smoothed_)
      {
        smoothed_ = sample;
        variation_ = sample / 2;
      }
      else
      {
        unsigned difference =
            sample > smoothed_ ? sample - smoothed_ : smoothed_ - sample;
        variation_ = (3 * variation_ + difference) / 4;
        smoothed_ = (7 * smoothed_ + sample) / 8;
      }
      unsigned deviation = 4 * variation_;
      interval_ = smoothed_ + (deviation > 100 ? deviation : 100);
      if (interval_ < 1000)
      {
        interval_ = 1000;
      }
      if (interval_ > 60000)
      {
        interval_ = 60000;
      }
      sampling_ = false;
    }
    if (!outstanding)
    {
      ambiguous_ = false;
    }
    start(now);
  }

private:

  unsigned smoothed_ = 0, variation_ = 0, interval_ = 1000, sample_end_ = 0;
  unsigned long started_ = 0, deadline_ = 0;
  bool sampling_ = false, ambiguous_ = false;
};
} // namespace xv::net
