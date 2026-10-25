export module xv.drivers:output;
export import xv.sync;

export namespace xv::drivers
{
// Process-context transmission may sleep. Echo uses the nonblocking put_sync.
class TerminalOutput
{
public:
  virtual int transmit(const char *data, int count) = 0;
  virtual void put_sync(int character) = 0;
  virtual void drain() = 0;
  virtual bool writable(xv::sync::PollTable * = nullptr) { return true; }
protected:
  constexpr TerminalOutput() = default;
  ~TerminalOutput() = default;
};
}
