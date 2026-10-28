export module xv.net:port;
export import :packet;
export import :address;
export import :configuration;

export namespace xv::net
{
class Layer2Port;
class BridgeManager;

class FrameReceiver
{
public:

  virtual unsigned bridge_id() const = 0;
  virtual void ingress(Layer2Port &port, unsigned epoch, Packet &frame) = 0;

protected:

  ~FrameReceiver() = default;
};

class Layer2Port
{
public:

  Layer2Port(const Layer2Port &) = delete;
  Layer2Port &operator=(const Layer2Port &) = delete;
  virtual unsigned frame_mtu() const = 0;
  virtual bool promiscuous(bool enable) = 0;
  virtual int transmit_frame(Packet &frame) = 0;

  unsigned master() const
  {
    ConfigurationGuard guard;
    return master_ ? master_->bridge_id() : 0;
  }

protected:

  constexpr Layer2Port() = default;
  ~Layer2Port() = default;

  bool dispatch_frame(Packet &frame)
  {
    FrameReceiver *receiver;
    unsigned epoch;
    {
      ConfigurationGuard guard;
      receiver = master_;
      epoch = epoch_;
    }
    if (!receiver)
    {
      return false;
    }
    receiver->ingress(*this, epoch, frame);
    return true;
  }

private:

  friend class BridgeManager;
  friend class BridgeInterface;
  FrameReceiver *master_ = nullptr;
  unsigned epoch_ = 0;
};
} // namespace xv::net
