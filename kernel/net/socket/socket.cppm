export module xv.net:socket;
export import :address;
export import xv.io;

export namespace xv::net
{
class Socket : public xv::io::IoObject, public xv::io::Capability
{
public:

  static constexpr xv::io::CapabilityId Id = xv::io::CapabilityId::Socket;

  xv::io::Capability *capability(xv::io::CapabilityId id) final
  {
    return id == Id ? this : nullptr;
  }

  int stat(xv::io::Metadata &value) final
  {
    value = {};
    value.type = static_cast<short>(xv::io::MetadataKind::Socket);
    value.nlink = 1;
    return 0;
  }

  virtual int bind(Endpoint local) = 0;
  virtual int connect(Endpoint peer, bool nonblocking) = 0;
  virtual int listen(int backlog) = 0;
  virtual Socket *accept(bool nonblocking, int &error) = 0;
  virtual int send_to(xv::io::Buffer data, unsigned length, const Endpoint *peer) = 0;
  virtual int receive_from(xv::io::Buffer data, unsigned length, Endpoint *peer) = 0;
  virtual int shutdown(int how) = 0;
  virtual Endpoint local() = 0;
  virtual Endpoint peer() = 0;
  virtual int error() = 0;

protected:

  constexpr Socket() = default;
  ~Socket() = default;
};

Socket *create_socket(int type, int protocol);
} // namespace xv::net
