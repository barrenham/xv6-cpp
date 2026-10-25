export module xv.trap:event;

export namespace xv::trap
{
using Word = unsigned long;
static_assert(sizeof(Word) == 8);

// RV64 scause contains an interrupt bit and an unsigned exception/IRQ code.
enum class CauseKind
{
  UserSystemCall,
  ExternalInterrupt,
  TimerInterrupt,
  LoadPageFault,
  StorePageFault,
  Other
};

class TrapCause final
{
public:

  static constexpr Word InterruptBit = Word{1} << 63;

  constexpr explicit TrapCause(Word value = 0) : value_(value)
  {
  }

  constexpr Word raw() const
  {
    return value_;
  }

  constexpr bool is_interrupt() const
  {
    return (value_ & InterruptBit) != 0;
  }

  constexpr Word code() const
  {
    return value_ & ~InterruptBit;
  }

  constexpr CauseKind kind() const
  {
    if (is_interrupt())
    {
      if (code() == 9)
      {
        return CauseKind::ExternalInterrupt;
      }
      if (code() == 5)
      {
        return CauseKind::TimerInterrupt;
      }
    }
    else
    {
      if (code() == 8)
      {
        return CauseKind::UserSystemCall;
      }
      if (code() == 13)
      {
        return CauseKind::LoadPageFault;
      }
      if (code() == 15)
      {
        return CauseKind::StorePageFault;
      }
    }
    return CauseKind::Other;
  }

private:

  Word value_;
};

// Captured before a handler can enable interrupts, block, or switch harts.
struct TrapEvent
{
  TrapCause cause;
  Word pc;
  Word status;
  Word value;
  static constexpr Word SupervisorPrevious = Word{1} << 8;
  static constexpr Word InterruptEnable = Word{1} << 1;

  constexpr bool from_supervisor() const
  {
    return (status & SupervisorPrevious) != 0;
  }

  constexpr bool interrupts_enabled() const
  {
    return (status & InterruptEnable) != 0;
  }
};
enum class TrapAction
{
  Resume,
  Yield,
  Terminate,
  Unhandled
};
} // namespace xv::trap
