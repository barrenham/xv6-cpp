#include <new>
#include "kernel/fs/userfs/protocol.h"
import xv.userfs.ramfs;

extern "C" void *
fs_test_backend_create(xv::userfs::Attributes *root) {
  auto *backend = new userfs::MemoryBackend(userfs::heap_storage());
  if (!backend->ready())
  {
    delete backend;
    return nullptr;
  }
  *root = backend->root();
  return backend;
}

extern "C" void
fs_test_backend_destroy(void *context) {
  delete static_cast<userfs::MemoryBackend *>(context);
}

extern "C" void
fs_test_backend_dispatch(void *context, xv::userfs::Message *message) {
  userfs::dispatch(*message, *static_cast<userfs::MemoryBackend *>(context));
}
