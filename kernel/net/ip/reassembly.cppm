export module xv.net:reassembly;
import :packet;
import xv.sync;

namespace xv::net
{
class Ipv4Reassembly final
{
public:

  static constexpr unsigned MaxPayload = 4062;
  void initialize();
  Packet accept(const void *ingress, const Packet &fragment, unsigned header_length,
                unsigned total_length, unsigned long now);
  void tick(unsigned long now);
  void clear();

private:

  struct AcceptFlow;

  struct Key
  {
    const void *ingress = nullptr;
    unsigned source = 0, destination = 0, identification = 0, protocol = 0;
  };

  struct Assembly
  {
    void *page = nullptr;
    Key key{};
    Packet::Context context{};
    unsigned total = 0, received = 0;
    bool last = false;
    unsigned char present[(MaxPayload + 7) / 8]{};
    unsigned long deadline = 0;
  } assemblies_[4]{};

  Assembly *find_or_create(const Key &key, const Packet &fragment, unsigned long now);
  static bool insert(Assembly &assembly, const unsigned char *payload, unsigned offset,
                     unsigned length, bool last);
  xv::sync::SpinLock lock_;
};
} // namespace xv::net
