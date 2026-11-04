module;
#include "kernel/support/types.h"
#include "kernel/fs/userfs/protocol.h"
#include "kernel/io/fs_control.h"
#include "user/fs/sdk/include/xvfs/plugin.h"
#include "user/fs/sdk/include/xvfs/control.h"
export module xv.userfs;

export namespace userfs
{
using xv::userfs::Attributes;
using xv::userfs::DataSize;
using xv::userfs::NameSize;

struct Entry
{
  Attributes attributes;
  unsigned long next = 0;
  char name[NameSize]{};
};

// The backend owns filesystem policy and storage. The library owns wire dispatch.
class Backend
{
public:

  virtual void
  context(const xv::userfs::RequestContext &) {
  }

  virtual int
  snapshot(unsigned, unsigned &) {
    return -1;
  }

  virtual int
  snapshot_read(unsigned long, void *, unsigned) {
    return -1;
  }

  virtual void
  snapshot_end() {
  }

  // Run one bounded batch between requests; no filesystem policy lives in the
  // dispatcher. A backend with pending work must eventually clear this flag.
  virtual bool
  maintenance_pending() const {
    return false;
  }

  virtual bool
  maintain() {
    return true;
  }

  // A deferred batch can request a bounded wait, interrupted by incoming RPCs.
  virtual unsigned
  maintenance_delay_ms() const {
    return 0;
  }

  unsigned
  payload_limit() const {
    return registered_payload_;
  }

  virtual const char *
  extension_interface() const {
    return "";
  }

  virtual unsigned
  extension_version() const {
    return 0;
  }

  // Input and output may alias. Access and identity originate in the kernel.
  virtual int
  control(unsigned node, const char *interface_name, unsigned version, unsigned command,
          unsigned access, const void *input, unsigned input_size, void *output,
          unsigned capacity) {
    return -1;
  }

  virtual Attributes
  root() {
    return {1, 0, 0, 0, 0755, 1, 2};
  }

  virtual unsigned
  flags() {
    return 0;
  }

  virtual unsigned long
  operations() {
    return xv::userfs::StandardOperations;
  }

  virtual int
  lookup(unsigned, const char *, Attributes &) {
    return -1;
  }

  virtual int
  getattr(unsigned, Attributes &) {
    return -1;
  }

  virtual int
  readdir(unsigned, unsigned long, Entry &) {
    return -1;
  }

  virtual int
  read(unsigned, unsigned long, void *, unsigned) {
    return -1;
  }

  virtual int
  write(unsigned, unsigned long, const void *, unsigned) {
    return -1;
  }

  virtual int
  create(unsigned, const char *, const Attributes &, Attributes &) {
    return -1;
  }

  virtual int
  resize(unsigned, unsigned) {
    return -1;
  }

  virtual int
  setattr(unsigned, const Attributes &) {
    return -1;
  }

  virtual int
  link(unsigned, const char *, unsigned) {
    return -1;
  }

  virtual int
  unlink(unsigned, const char *, Attributes &) {
    return -1;
  }

  virtual int
  rename(unsigned, const char *, unsigned, const char *) {
    return -1;
  }

  virtual int
  sync(unsigned) {
    return 0;
  }

  virtual void
  forget(unsigned) {
  }

private:

  friend int register_backend(const char *, Backend &, unsigned);
  unsigned registered_payload_ = DataSize;

protected:

  ~Backend() = default;
};

int register_backend(const char *type, Backend &backend, unsigned payload = DataSize);
void dispatch(xv::userfs::Message &, Backend &);
int serve(int descriptor, Backend &backend);
} // namespace userfs

static_assert(sizeof(xvfs_attributes) == sizeof(xv::userfs::Attributes));
static_assert(sizeof(xvfs_context) == sizeof(xv::userfs::RequestContext));
static_assert(sizeof(xvfs_message) == sizeof(xv::userfs::Message));
static_assert(sizeof(xvfs_registration) == sizeof(xv::userfs::Registration));
static_assert(offsetof(xvfs_message, version) ==
              offsetof(xv::userfs::Message, version));
static_assert(offsetof(xvfs_message, operation) ==
              offsetof(xv::userfs::Message, operation));
static_assert(offsetof(xvfs_message, id) == offsetof(xv::userfs::Message, id));
static_assert(offsetof(xvfs_message, node) == offsetof(xv::userfs::Message, node));
static_assert(offsetof(xvfs_message, other) == offsetof(xv::userfs::Message, other));
static_assert(offsetof(xvfs_message, offset) == offsetof(xv::userfs::Message, offset));
static_assert(offsetof(xvfs_message, result) == offsetof(xv::userfs::Message, result));
static_assert(offsetof(xvfs_message, count) == offsetof(xv::userfs::Message, count));
static_assert(offsetof(xvfs_message, context) ==
              offsetof(xv::userfs::Message, context));
static_assert(offsetof(xvfs_message, attributes) ==
              offsetof(xv::userfs::Message, attributes));
static_assert(offsetof(xvfs_message, name) == offsetof(xv::userfs::Message, name));
static_assert(offsetof(xvfs_message, target) == offsetof(xv::userfs::Message, target));
static_assert(offsetof(xvfs_message, data) == offsetof(xv::userfs::Message, data));
static_assert(offsetof(xvfs_registration, size) ==
              offsetof(xv::userfs::Registration, size));
static_assert(offsetof(xvfs_registration, version) ==
              offsetof(xv::userfs::Registration, version));
static_assert(offsetof(xvfs_registration, flags) ==
              offsetof(xv::userfs::Registration, flags));
static_assert(offsetof(xvfs_registration, max_payload) ==
              offsetof(xv::userfs::Registration, max_payload));
static_assert(offsetof(xvfs_registration, operations) ==
              offsetof(xv::userfs::Registration, operations));
static_assert(offsetof(xvfs_registration, type) ==
              offsetof(xv::userfs::Registration, type));
static_assert(offsetof(xvfs_registration, root) ==
              offsetof(xv::userfs::Registration, root));
static_assert(XVFS_WIRE_VERSION == xv::userfs::Version);
static_assert(XVFS_READ_OPERATIONS == xv::userfs::ReadOperations);

static_assert(sizeof(xvfs_control) == sizeof(xv::fscontrol::Request));
static_assert(XVFS_CONTROL_IOCTL == xv::fscontrol::Ioctl);
static_assert(XVFS_CONTROL == static_cast<unsigned>(xv::userfs::Operation::Control));
static_assert(offsetof(xvfs_control, size) == offsetof(xv::fscontrol::Request, size));
static_assert(offsetof(xvfs_control, version) ==
              offsetof(xv::fscontrol::Request, version));
static_assert(offsetof(xvfs_control, driver_version) ==
              offsetof(xv::fscontrol::Request, driver_version));
static_assert(offsetof(xvfs_control, command) ==
              offsetof(xv::fscontrol::Request, command));
static_assert(offsetof(xvfs_control, input_size) ==
              offsetof(xv::fscontrol::Request, input_size));
static_assert(offsetof(xvfs_control, capacity) ==
              offsetof(xv::fscontrol::Request, capacity));
static_assert(offsetof(xvfs_control, output_size) ==
              offsetof(xv::fscontrol::Request, output_size));
static_assert(offsetof(xvfs_control, interface_name) ==
              offsetof(xv::fscontrol::Request, interface_name));
static_assert(offsetof(xvfs_control, data) == offsetof(xv::fscontrol::Request, data));
