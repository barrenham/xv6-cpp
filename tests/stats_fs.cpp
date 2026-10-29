module xv.fs;
namespace xv::fs
{
namespace
{
constinit DeviceRegistry registry;
xv::drivers::CharacterDevice *attached = nullptr;
unsigned major_number = 0;
} // namespace
DeviceRegistry &devices() { return registry; }
void DeviceRegistry::attach(unsigned major, xv::drivers::CharacterDevice &device)
{
  major_number = major;
  attached = &device;
}
extern "C" void *stats_attached_device() { return attached; }
extern "C" unsigned stats_attached_major() { return major_number; }
} // namespace xv::fs
