module;
#include <string>
module xv.drivers;

namespace xv::drivers
{
namespace
{
std::string output;

class Capture final : public CharacterDevice
{
public:

  const char *name() const override
  {
    return "print capture";
  }

  void initialize() override
  {
  }

  int read(IoBuffer, int) override
  {
    return -1;
  }

  int write(IoBuffer, int) override
  {
    return -1;
  }

  void put_sync(int character) override
  {
    output.push_back(static_cast<char>(character));
  }
} capture;
} // namespace

CharacterDevice &console()
{
  return capture;
}

extern "C" const char *captured_print()
{
  return output.c_str();
}

extern "C" void reset_print()
{
  output.clear();
}
} // namespace xv::drivers
