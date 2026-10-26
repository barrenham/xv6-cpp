module xv.fs:operations;
import :file;
import :pipe;
namespace xv::fs
{
class InodeBackend final : public xv::io::IoObject
{
public:
  explicit InodeBackend(Inode &node) : node_(&node) {}
  void close() override;
  int read(xv::io::Buffer destination, int count) override;
  int write(xv::io::Buffer source, int count) override;
  int stat(xv::io::Metadata &value) override;
  unsigned poll(xv::sync::PollTable *table) override;
private:
  Inode *node_;
  unsigned offset_ = 0;
};
class DeviceBackend final : public xv::io::IoObject, public xv::io::DeviceControl
{
public:
  explicit DeviceBackend(Inode &node) : node_(&node), major_(node.device_major()) {}
  void close() override;
  int read(xv::io::Buffer destination, int count) override;
  int write(xv::io::Buffer source, int count) override;
  int stat(xv::io::Metadata &value) override;
  unsigned poll(xv::sync::PollTable *table) override;
  int ioctl(unsigned request, xv::io::Buffer argument) override;
  xv::io::Capability *capability(xv::io::CapabilityId id) override
  { return id == xv::io::DeviceControl::Id ? static_cast<xv::io::DeviceControl *>(this) : nullptr; }
private:
  Inode *node_;
  short major_;
};
class PipeBackend final : public xv::io::IoObject
{
public:
  PipeBackend(Pipe &pipe, bool writable) : pipe_(&pipe), writable_(writable) {}
  void close() override;
  int read(xv::io::Buffer destination, int count) override;
  int write(xv::io::Buffer source, int count) override;
  unsigned poll(xv::sync::PollTable *table) override;
private:
  Pipe *pipe_;
  bool writable_;
};
class EndpointBackend final : public xv::io::IoObject, public xv::io::DeviceControl
{
public:
  explicit EndpointBackend(xv::drivers::CharacterDevice &endpoint) : endpoint_(&endpoint) {}
  void close() override;
  int read(xv::io::Buffer destination, int count) override;
  int write(xv::io::Buffer source, int count) override;
  int stat(xv::io::Metadata &value) override;
  unsigned poll(xv::sync::PollTable *table) override;
  int ioctl(unsigned request, xv::io::Buffer argument) override;
  xv::io::Capability *capability(xv::io::CapabilityId id) override
  { return id == xv::io::DeviceControl::Id ? static_cast<xv::io::DeviceControl *>(this) : nullptr; }
private:
  xv::drivers::CharacterDevice *endpoint_;
};
}
