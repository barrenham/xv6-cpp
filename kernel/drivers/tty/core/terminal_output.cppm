export module xv.drivers:output;
export import xv.sync;

export namespace xv::drivers
{
// Process-context transmit accepts a complete chunk or returns a negative status.
// nonblocking forbids sleeping; echo uses the IRQ-safe synchronous put_sync path.
class TerminalOutput
{
public:
  virtual int transmit(const char *data, int count, bool nonblocking = false) = 0;
  virtual void put_sync(int character) = 0;
  virtual void drain() = 0;
  virtual bool writable(xv::sync::PollTable * = nullptr) { return true; }
protected:
  constexpr TerminalOutput() = default;
  ~TerminalOutput() = default;
};
}
