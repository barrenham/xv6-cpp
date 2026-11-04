// Included after importing the device interfaces, inside the driver's module.
namespace xv::drivers::plugin
{
class PollAdapter final : public xv::sync::PollTable
{
public:

  explicit PollAdapter(void *token) : token_(token) {
    if (token)
    {
      request(xv_driver_host()->poll_events(token));
    }
  }

  void
  watch(void *channel) override {
    if (token_)
    {
      xv_driver_host()->poll_watch(token_, channel);
    }
  }

private:

  void *token_;
};

IoBuffer
buffer(xv_kmod_buffer value) {
  return {value.user ? AddressSpace::User : AddressSpace::Kernel, value.address,
          value.nonblocking};
}

void *
open(void *context, bool nonblocking) {
  return static_cast<CharacterDevice *>(context)->open(nonblocking);
}

void
close(void *context) {
  static_cast<CharacterDevice *>(context)->close();
}

int
read(void *context, xv_kmod_buffer destination, int count) {
  return static_cast<CharacterDevice *>(context)->read(buffer(destination), count);
}

int
write(void *context, xv_kmod_buffer source, int count) {
  return static_cast<CharacterDevice *>(context)->write(buffer(source), count);
}

int
ioctl(void *context, unsigned request, xv_kmod_buffer argument) {
  return static_cast<CharacterDevice *>(context)->ioctl(request, buffer(argument));
}

unsigned
poll(void *context, void *table) {
  PollAdapter adapter(table);
  return static_cast<CharacterDevice *>(context)->poll(table ? &adapter : nullptr);
}

xv_kmod_character
character(CharacterDevice &device) {
  return {&device, open, close, read, write, ioctl, poll};
}
} // namespace xv::drivers::plugin
