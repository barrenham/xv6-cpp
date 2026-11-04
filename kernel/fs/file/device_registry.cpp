module xv.fs;
import :file;

namespace xv::fs
{
void
initialize_device_registry() {
  // Physical and software device backends have already registered from ELF.
  devices().seal();
}
} // namespace xv::fs
