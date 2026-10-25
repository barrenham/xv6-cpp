export module xv.drivers:serial;

export import :interrupt;
export import :output;

export namespace xv::drivers
{
class InputReceiver
{
public:

  virtual void receive(int character) = 0;

protected:

  constexpr InputReceiver() = default;
  ~InputReceiver() = default;
};

class SerialPort : public InterruptDevice, public TerminalOutput
{
public:

  // Attach before enabling RX interrupts; the receiver must outlive the port.
  virtual void attach(InputReceiver &receiver) = 0;
  // Process context only; may block until the transmitter becomes ready.
  virtual void write(const char *data, int count) = 0;
  int transmit(const char *data, int count) override
  {
    write(data, count);
    return count;
  }
  // Polling path for early boot, interrupt-time echo, and panic output.
  void put_sync(int character) override = 0;
  // Wait until the hardware shift register is empty (process context).
  void drain() override = 0;

protected:

  constexpr SerialPort() = default;
  ~SerialPort() = default;
};
} // namespace xv::drivers
