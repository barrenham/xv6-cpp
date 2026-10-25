export module xv.drivers:interrupt;

export import :device;

export namespace xv::drivers
{
class InterruptDevice : public Device
{
public:

  virtual unsigned irq() const = 0;
  virtual void handle_interrupt() = 0;

protected:

  constexpr InterruptDevice() = default;
  ~InterruptDevice() = default;
};

class InterruptController : public Device
{
public:

  virtual void set_priority(unsigned irq, unsigned priority) = 0;
  virtual void initialize_hart(int hart) = 0;
  virtual void enable(int hart, unsigned irq) = 0;
  virtual unsigned claim(int hart) = 0;
  virtual void complete(int hart, unsigned irq) = 0;

protected:

  constexpr InterruptController() = default;
  ~InterruptController() = default;
};
} // namespace xv::drivers
