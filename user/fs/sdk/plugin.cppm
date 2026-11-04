export module xv.userfs.plugin;
export import xv.userfs;

export namespace userfs::plugin
{
inline constexpr unsigned Abi = 1;
inline constexpr int Source = 3, Resources = 4, Ready = 5, Activation = 6, Image = 7;

struct Invocation
{
  bool valid = false, probe = false;
  const char *type = nullptr, *options = nullptr;
  unsigned mode = 0755;
  Invocation(int argc, char **argv);
};

// Registers, reports readiness, waits for publication, and serves until EOF.
// Implementations own their storage and resource descriptors throughout.
int run(const Invocation &, Backend &);
}
