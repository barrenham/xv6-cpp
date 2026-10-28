export module xv.net:filter_hook;

export namespace xv::net
{
enum class FilterHook : unsigned
{
  Input,
  Output
};

// The IPv4 layer knows only this interface, never rules or control operations.
class PacketFilter
{
public:

  virtual bool accept(FilterHook hook, const unsigned char *data, unsigned size,
                      unsigned interface) = 0;

protected:

  ~PacketFilter() = default;
};
} // namespace xv::net
