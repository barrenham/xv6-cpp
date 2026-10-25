module xv.trap:handlers;
import :dispatch;
import :clock;

namespace xv::trap
{
class SystemCallHandler final : public TrapHandler
{
public:

  constexpr SystemCallHandler() = default;

  const char *name() const override
  {
    return "user system call";
  }

  bool accepts(const TrapEvent &event) const override;
  TrapAction handle(const TrapEvent &event) override;
};

class ExternalInterruptHandler final : public TrapHandler
{
public:

  constexpr ExternalInterruptHandler() = default;

  const char *name() const override
  {
    return "supervisor external interrupt";
  }

  bool accepts(const TrapEvent &event) const override;
  TrapAction handle(const TrapEvent &event) override;
};

class TimerInterruptHandler final : public TrapHandler
{
public:

  constexpr explicit TimerInterruptHandler(TickClock &clock) : clock_(clock)
  {
  }

  const char *name() const override
  {
    return "supervisor timer interrupt";
  }

  bool accepts(const TrapEvent &event) const override;
  TrapAction handle(const TrapEvent &event) override;

private:

  TickClock &clock_;
};

class PageFaultHandler final : public TrapHandler
{
public:

  constexpr PageFaultHandler() = default;

  const char *name() const override
  {
    return "user data page fault";
  }

  bool accepts(const TrapEvent &event) const override;
  TrapAction handle(const TrapEvent &event) override;
};
} // namespace xv::trap
