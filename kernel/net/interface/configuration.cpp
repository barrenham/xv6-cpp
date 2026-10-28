module xv.net;
import :configuration;

namespace xv::net
{
namespace
{
constinit xv::sync::SpinLock config_lock;
}

xv::sync::SpinLock &configuration_lock()
{
  return config_lock;
}
} // namespace xv::net
