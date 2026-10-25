export module xv.drivers:serial;

export import :interrupt;

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

class SerialPort : public InterruptDevice
{
public:

  // Attach before enabling RX interrupts; the receiver must outlive the port.
  virtual void attach(InputReceiver &receiver) = 0;
  // Process context only; may block until the transmitter becomes ready.
  virtual void write(const char *data, int count) = 0;
  // Polling path for early boot, interrupt-time echo, and panic output.
  virtual void put_sync(int character) = 0;
  // Wait until the hardware shift register is empty (process context).
  virtual void drain() = 0;

protected:

  constexpr SerialPort() = default;
  ~SerialPort() = default;
};
} // namespace xv::drivers
