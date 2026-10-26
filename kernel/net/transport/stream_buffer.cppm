module xv.net:stream_buffer;
import :platform;
import :wire;
namespace xv::net
{
class TcpPageBuffer final
{
public:
  static constexpr unsigned Capacity = 16384;
  bool initialize()
  {
    for (auto &page : pages_)
      if (!page && !(page = static_cast<unsigned char *>(detail::allocate_page()))) return false;
    return true;
  }
  void release()
  { for (auto &page : pages_) if (page) { detail::free_page(page); page = nullptr; } }
  unsigned char &at(unsigned sequence) { return pages_[(sequence & (Capacity-1))/4096][sequence%4096]; }
  const unsigned char *span(unsigned sequence, unsigned &length) const
  { unsigned remaining = 4096-sequence%4096; if (length > remaining) length = remaining; return pages_[(sequence & (Capacity-1))/4096]+sequence%4096; }
private:
  unsigned char *pages_[4]{};
};
class TcpSendBuffer final
{
public:
  bool initialize() { return data_.initialize(); }
  void release() { data_.release(); }
  void reset(unsigned sequence) { begin_ = end_ = sequence; }
  unsigned begin() const { return begin_; }
  unsigned end() const { return end_; }
  unsigned size() const { return end_-begin_; }
  unsigned space() const { return TcpPageBuffer::Capacity-size(); }
  unsigned append(const unsigned char *data, unsigned length)
  { if (length > space()) length = space(); for (unsigned i = 0; i < length; ++i) data_.at(end_++) = data[i]; return length; }
  unsigned discard(unsigned acknowledgement)
  {
    if (!wire::before(begin_, acknowledgement)) return 0;
    unsigned count = wire::before(end_, acknowledgement) ? size() : acknowledgement-begin_;
    begin_ += count; return count;
  }
  const unsigned char *span(unsigned sequence, unsigned &length) const { return data_.span(sequence, length); }
private:
  TcpPageBuffer data_;
  unsigned begin_ = 0, end_ = 0;
};
class TcpReceiveBuffer final
{
public:
  bool initialize() { return data_.initialize(); }
  void release() { data_.release(); }
  void reset(unsigned sequence)
  { next_ = read_ = sequence; ready_ = 0; finished_ = fin_pending_ = false; wire::zero(present_, sizeof(present_)); }
  unsigned next() const { return next_; }
  unsigned readable() const { return ready_; }
  unsigned window() const { return TcpPageBuffer::Capacity-ready_; }
  bool finished() const { return finished_; }
  bool acceptable(unsigned sequence, unsigned length) const
  {
    unsigned available = window();
    if (!length) return available ? !wire::before(sequence,next_) && wire::before(sequence,next_+available) : sequence == next_;
    return available && wire::before(sequence,next_+available) && wire::before(next_,sequence+length);
  }
  unsigned insert(unsigned sequence, const unsigned char *data, unsigned length, bool fin)
  {
    if (finished_) return 0;
    unsigned old = next_, right = next_+window(), ending = sequence+length;
    if (fin && !wire::before(ending,next_) && wire::before(ending,right) && (!fin_pending_ || wire::before(ending,fin_sequence_)))
    { fin_pending_ = true; fin_sequence_ = ending; }
    for (unsigned i = 0; i < length; ++i)
    {
      unsigned position = sequence+i;
      if (wire::before(position,next_) || !wire::before(position,right) || (fin_pending_ && !wire::before(position,fin_sequence_))) continue;
      unsigned slot = position & (TcpPageBuffer::Capacity-1), mask = 1U << (slot%8);
      if (!(present_[slot/8]&mask)) { data_.at(position) = data[i]; present_[slot/8] |= mask; }
    }
    for (;;)
    {
      if (fin_pending_ && next_ == fin_sequence_) { finished_ = true; ++next_; break; }
      unsigned slot = next_ & (TcpPageBuffer::Capacity-1), mask = 1U << (slot%8);
      if (!(present_[slot/8]&mask)) break;
      present_[slot/8] &= ~mask; ++next_; ++ready_;
    }
    return next_-old;
  }
  unsigned read(unsigned char *out, unsigned length)
  { if (length > ready_) length = ready_; for (unsigned i = 0; i < length; ++i) out[i] = data_.at(read_++); ready_ -= length; return length; }
  void discard() { read_ += ready_; ready_ = 0; }
private:
  TcpPageBuffer data_;
  unsigned next_ = 0, read_ = 0, ready_ = 0, fin_sequence_ = 0;
  unsigned char present_[TcpPageBuffer::Capacity/8]{};
  bool fin_pending_ = false, finished_ = false;
};
}
