export module xv.net:dhcp_message;
export import :address;

export namespace xv::net
{
struct DhcpLease
{
  unsigned address = 0, prefix = 0, gateway = 0, dns = 0, server = 0;
  unsigned seconds = 0, renewal = 0, rebinding = 0;
};

struct DhcpReply
{
  unsigned type = 0, xid = 0, address = 0, server = 0, mask = 0, gateway = 0, dns = 0;
  unsigned seconds = 0, renewal = 0, rebinding = 0, present = 0;
  MacAddress mac{};
};

struct DhcpRequest
{
  unsigned type = 0, xid = 0, address = 0, requested = 0, server = 0, elapsed = 0;
  MacAddress mac{};
  bool broadcast = true;
};

class DhcpCodec final
{
public:

  static unsigned encode(unsigned char *buffer, unsigned capacity,
                         const DhcpRequest &request);
  static bool decode(const unsigned char *buffer, unsigned length, DhcpReply &reply);
  static bool lease(const DhcpReply &reply, DhcpLease &result);

private:

  struct OptionsFlow;

  struct DecodeFlow;
  struct EncodeFlow;
  struct LeaseFlow;

  static bool options(const unsigned char *data, unsigned length, DhcpReply &reply,
                      unsigned &overload, bool main);
};
} // namespace xv::net
