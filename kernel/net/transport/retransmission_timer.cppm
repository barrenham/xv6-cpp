module xv.net:retransmission_timer;
namespace xv::net
{
class RetransmissionTimer final
{
public:
  void start(unsigned long now)
  { started_ = now; deadline_ = now+interval_; retransmitted_ = false; }
  bool expired(unsigned long now) const { return now >= deadline_; }
  void retransmit(unsigned long now)
  {
    retransmitted_ = true;
    interval_ = interval_ < 30000 ? interval_*2 : 60000;
    deadline_ = now+interval_;
  }
  void restart(unsigned long now) { deadline_ = now+interval_; retransmitted_ = true; }
  void acknowledge(unsigned long now)
  {
    if (retransmitted_) return;
    unsigned sample = now-started_; if (!sample) sample = 1;
    if (!smoothed_) { smoothed_ = sample; variation_ = sample/2; }
    else
    {
      unsigned difference = sample > smoothed_ ? sample-smoothed_ : smoothed_-sample;
      variation_ = (3*variation_+difference)/4;
      smoothed_ = (7*smoothed_+sample)/8;
    }
    unsigned deviation = 4*variation_;
    interval_ = smoothed_+(deviation > 100 ? deviation : 100);
    if (interval_ < 1000) interval_ = 1000;
    if (interval_ > 60000) interval_ = 60000;
  }
private:
  unsigned smoothed_ = 0, variation_ = 0, interval_ = 1000;
  unsigned long started_ = 0, deadline_ = 0;
  bool retransmitted_ = false;
};
}
