export module xv.net:configuration;
import xv.sync;

export namespace xv::net
{
xv::sync::SpinLock &configuration_lock();

class ConfigurationGuard final
{
public:

  ConfigurationGuard()
  {
    xv::sync::push_interrupts_off();
    acquired_ = !configuration_lock().holding();
    if (acquired_)
    {
      configuration_lock().acquire();
    }
  }

  ~ConfigurationGuard()
  {
    if (acquired_)
    {
      configuration_lock().release();
    }
    xv::sync::pop_interrupts_off();
  }

  ConfigurationGuard(const ConfigurationGuard &) = delete;
  ConfigurationGuard &operator=(const ConfigurationGuard &) = delete;

private:

  bool acquired_;
};
} // namespace xv::net
