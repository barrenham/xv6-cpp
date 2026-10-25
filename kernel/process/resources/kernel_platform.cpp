module xv.process;

import :resource_platform;
import xv.fs;
import xv.print;

namespace xv::process::detail
{
xv::fs::File *retain_file(xv::fs::File *file)
{
  return xv::fs::files().retain(file);
}

void release_file(xv::fs::File *file)
{
  xv::fs::files().release(file);
}

xv::fs::Inode *retain_inode(xv::fs::Inode *node)
{
  return xv::fs::filesystem().retain(node);
}

void release_inode(xv::fs::Inode *node)
{
  xv::fs::filesystem().release(node);
}

[[noreturn]] void resource_fail(const char *message)
{
  xv::print::panic(message);
}
} // namespace xv::process::detail
