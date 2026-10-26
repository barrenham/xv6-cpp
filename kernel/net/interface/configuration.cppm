export module xv.net:configuration;
import xv.sync;
export namespace xv::net
{
xv::sync::SpinLock &configuration_lock();
class ConfigurationGuard final
{
public:
  ConfigurationGuard() : acquired_(!configuration_lock().holding())
  { if (acquired_) configuration_lock().acquire(); }
  ~ConfigurationGuard() { if (acquired_) configuration_lock().release(); }
  ConfigurationGuard(const ConfigurationGuard &) = delete;
  ConfigurationGuard &operator=(const ConfigurationGuard &) = delete;
private:
  bool acquired_;
};
}
