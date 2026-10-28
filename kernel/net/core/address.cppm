export module xv.net:address;

export namespace xv::net
{
struct Endpoint
{
  unsigned address = 0;
  unsigned short port = 0;
};

struct MacAddress
{
  unsigned char bytes[6]{};
};

struct Ipv4Address
{
  unsigned address = 0;
  unsigned prefix = 0;
  unsigned owner = 0;

  unsigned mask() const
  {
    return prefix && prefix <= 32 ? ~0U << (32 - prefix) : 0;
  }

  bool contains(unsigned value) const
  {
    return (value & mask()) == (address & mask());
  }
};
} // namespace xv::net
