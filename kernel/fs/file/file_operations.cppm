module xv.fs:operations;
import :file;
import :pipe;

namespace xv::fs
{
class InodeBackend final : public xv::io::IoObject
{
public:

  InodeBackend(Inode &node, bool snapshot, PageBuffer &contents)
      : node_(&node), snapshot_(snapshot)
  {
    contents_.take(contents);
  }

  void close() override;
  int read(xv::io::Buffer destination, int count) override;
  int write(xv::io::Buffer source, int count) override;
  long seek(long offset, int origin) override;
  int resize(unsigned size) override;
  int sync() override;
  int stat(xv::io::Metadata &value) override;
  unsigned poll(xv::sync::PollTable *table) override;

private:

  struct MutationProtocol;
  struct ReadFlow;
  struct WriteFlow;
  struct SeekFlow;
  struct ResizeFlow;
  struct SyncFlow;
  Inode *node_;
  unsigned offset_ = 0;
  bool snapshot_ = false;
  PageBuffer contents_;
};

class DeviceBackend final : public xv::io::IoObject, public xv::io::DeviceControl
{
public:

  DeviceBackend(Inode &node, xv::drivers::CharacterDevice &session)
      : node_(&node), session_(&session)
  {
  }

  void close() override;
  int read(xv::io::Buffer destination, int count) override;
  int write(xv::io::Buffer source, int count) override;
  int stat(xv::io::Metadata &value) override;
  unsigned poll(xv::sync::PollTable *table) override;
  int ioctl(unsigned request, xv::io::Buffer argument) override;

  xv::io::Capability *capability(xv::io::CapabilityId id) override
  {
    return id == xv::io::DeviceControl::Id ? static_cast<xv::io::DeviceControl *>(this)
                                           : nullptr;
  }

private:

  Inode *node_;
  xv::drivers::CharacterDevice *session_;
};

class PipeBackend final : public xv::io::IoObject
{
public:

  PipeBackend(Pipe &pipe, bool writable) : pipe_(&pipe), writable_(writable)
  {
  }

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

  explicit EndpointBackend(xv::drivers::CharacterDevice &endpoint)
      : endpoint_(&endpoint)
  {
  }

  void close() override;
  int read(xv::io::Buffer destination, int count) override;
  int write(xv::io::Buffer source, int count) override;
  int stat(xv::io::Metadata &value) override;
  unsigned poll(xv::sync::PollTable *table) override;
  int ioctl(unsigned request, xv::io::Buffer argument) override;

  xv::io::Capability *capability(xv::io::CapabilityId id) override
  {
    return id == xv::io::DeviceControl::Id ? static_cast<xv::io::DeviceControl *>(this)
                                           : nullptr;
  }

private:

  xv::drivers::CharacterDevice *endpoint_;
};
} // namespace xv::fs
