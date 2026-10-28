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

  bool duplicate(unsigned acknowledgement, unsigned flight, unsigned next)
  {
    if (!flight || (barrier_ && wire::before(acknowledgement, recover_)))
    {
      return false;
    }
    ++duplicates_;
    if (duplicates_ == 3)
    {
      threshold_ = flight / 2 > 2 * mss_ ? flight / 2 : 2 * mss_;
      window_ = threshold_ + 3 * mss_;
      recover_ = next;
      recovery_ = true;
      return true;
    }
    if (recovery_ && duplicates_ > 3 && window_ < 65535 - mss_)
    {
      window_ += mss_;
    }
    return false;
  }

  bool acknowledged(unsigned bytes, unsigned acknowledgement)
  {
    if (!bytes)
    {
      return false;
    }
    if (recovery_)
    {
      if (wire::before(acknowledgement, recover_))
      {
        window_ = window_ > bytes ? window_ - bytes : mss_;
        if (bytes >= mss_)
        {
          window_ += mss_;
        }
        return true;
      }
      recovery_ = false;
      barrier_ = false;
      window_ = threshold_;
      credit_ = duplicates_ = 0;
      return false;
    }
    duplicates_ = 0;
    if (barrier_ && !wire::before(acknowledgement, recover_))
    {
      barrier_ = false;
    }
    if (window_ < threshold_)
    {
      window_ += bytes < mss_ ? bytes : mss_;
    }
    else
    {
      credit_ += bytes;
      if (credit_ >= window_)
      {
        credit_ -= window_;
        window_ += mss_;
      }
    }
    if (window_ > 65535)
    {
      window_ = 65535;
    }
    return false;
  }

  void reset_duplicates()
  {
    if (!recovery_)
    {
      duplicates_ = 0;
    }
  }

private:

  unsigned mss_ = 536, window_ = 1072, threshold_ = 65535, duplicates_ = 0, credit_ = 0,
           recover_ = 0;
  bool recovery_ = false, barrier_ = false;
};
} // namespace xv::net
