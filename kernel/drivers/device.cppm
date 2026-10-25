export module xv.drivers:device;

export namespace xv::drivers
{
class Device
{
public:

  Device(const Device &) = delete;
  Device &operator=(const Device &) = delete;
  virtual const char *name() const = 0;
  // Boot-only initialization, before enabling device interrupts on any hart.
  virtual void initialize() = 0;

protected:

  constexpr Device() = default;
  ~Device() = default;
};
} // namespace xv::drivers
