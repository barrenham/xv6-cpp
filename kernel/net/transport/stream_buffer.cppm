module xv.net:stream_buffer;
import :platform;
import :wire;

namespace xv::net
{
class TcpPageBuffer final
{
public:

  static constexpr unsigned Capacity = 16384;

  bool initialize();

  void release()
  {
    for (auto &page : pages_)
    {
      if (page)
      {
        detail::free_page(page);
        page = nullptr;
      }
    }
  }

  unsigned char &at(unsigned sequence)
  {
    return pages_[(sequence & (Capacity - 1)) / 4096][sequence % 4096];
  }

  const unsigned char *span(unsigned sequence, unsigned &length) const
  {
    unsigned remaining = 4096 - sequence % 4096;
    if (length > remaining)
    {
      length = remaining;
    }
    return pages_[(sequence & (Capacity - 1)) / 4096] + sequence % 4096;
  }

private:

  struct InitializeFlow;

  unsigned char *pages_[4]{};
};

class TcpSendBuffer final
{
public:

  bool initialize()
  {
    return data_.initialize();
  }

  void release()
  {
    data_.release();
  }

  void reset(unsigned sequence)
  {
    begin_ = end_ = sequence;
  }

  unsigned begin() const
  {
    return begin_;
  }

  unsigned end() const
  {
    return end_;
  }

  unsigned size() const
  {
    return end_ - begin_;
  }

  unsigned space() const
  {
    return TcpPageBuffer::Capacity - size();
  }

  unsigned append(const unsigned char *data, unsigned length);

  unsigned discard(unsigned acknowledgement)
  {
    if (!wire::before(begin_, acknowledgement))
    {
      return 0;
    }
    unsigned count =
        wire::before(end_, acknowledgement) ? size() : acknowledgement - begin_;
    begin_ += count;
    return count;
  }

  const unsigned char *span(unsigned sequence, unsigned &length) const
  {
    return data_.span(sequence, length);
  }

private:

  struct AppendFlow;

  TcpPageBuffer data_;
  unsigned begin_ = 0, end_ = 0;
};

class TcpReceiveBuffer final
{
public:

  bool initialize()
  {
    return data_.initialize();
  }

  void release()
  {
    data_.release();
  }

  void reset(unsigned sequence)
  {
    next_ = read_ = sequence;
    ready_ = 0;
    finished_ = fin_pending_ = false;
    wire::zero(present_, sizeof(present_));
  }

  unsigned next() const
  {
    return next_;
  }

  unsigned readable() const
  {
    return ready_;
  }

  unsigned window() const
  {
    return TcpPageBuffer::Capacity - ready_;
  }

  bool finished() const
  {
    return finished_;
  }

  bool acceptable(unsigned sequence, unsigned length) const
  {
    unsigned available = window();
    if (!length)
    {
      return available ? !wire::before(sequence, next_) &&
                             wire::before(sequence, next_ + available)
                       : sequence == next_;
    }
    return available && wire::before(sequence, next_ + available) &&
           wire::before(next_, sequence + length);
  }

  unsigned insert(unsigned sequence, const unsigned char *data, unsigned length,
                  bool fin);

  unsigned read(unsigned char *out, unsigned length)
  {
    if (length > ready_)
    {
      length = ready_;
    }
    for (unsigned i = 0; i < length; ++i)
    {
      out[i] = data_.at(read_++);
    }
    ready_ -= length;
    return length;
  }

  void discard()
  {
    read_ += ready_;
    ready_ = 0;
  }

private:

  struct InsertFlow;

  TcpPageBuffer data_;
  unsigned next_ = 0, read_ = 0, ready_ = 0, fin_sequence_ = 0;
  unsigned char present_[TcpPageBuffer::Capacity / 8]{};
  bool fin_pending_ = false, finished_ = false;
};
} // namespace xv::net
