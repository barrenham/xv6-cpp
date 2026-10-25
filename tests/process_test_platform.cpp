module;
#include "tests/process_test_hooks.h"

module xv.process;
import :resource_platform;

namespace xv::process::detail
{
xv::fs::File *retain_file(xv::fs::File *file)
{
  return static_cast<xv::fs::File *>(test_retain_file(file));
}

void release_file(xv::fs::File *file)
{
  test_release_file(file);
}

xv::fs::Inode *retain_inode(xv::fs::Inode *node)
{
  return static_cast<xv::fs::Inode *>(test_retain_inode(node));
}

void release_inode(xv::fs::Inode *node)
{
  test_release_inode(node);
}

[[noreturn]] void resource_fail(const char *message)
{
  test_resource_fail(message);
}
} // namespace xv::process::detail
