#pragma once
#include "server.h"
#include "resources.h"

namespace userfs
{
class DeviceBackend final : public Backend
{
public:

  explicit DeviceBackend(int resources) : resources_(resources) {}
  unsigned flags() override { return xv::userfs::ReadOnly; }
  int lookup(unsigned, const char *, Attributes &) override;
  int getattr(unsigned, Attributes &) override;
  int readdir(unsigned, unsigned long, Entry &) override;
  int read(unsigned, unsigned long, void *, unsigned) override;
  void forget(unsigned) override;

private:

  static constexpr unsigned Root = 1, Pts = 2, DeviceBase = 16, PtyBase = 256;
  static bool decimal(const char *, unsigned &);
  static void name(unsigned, char *);
  bool device(unsigned, Attributes &, char * = nullptr);
  bool pty(unsigned, Attributes &, bool);
  Resources resources_;
};
} // namespace userfs
