module xv.process:resource_platform;

import xv.fs;

namespace xv::process::detail
{
xv::fs::File *retain_file(xv::fs::File *file);
void release_file(xv::fs::File *file);
bool file_info(xv::fs::File *file, xv::fs::FileInfo &result);
xv::fs::Inode *retain_inode(xv::fs::Inode *node);
void release_inode(xv::fs::Inode *node);
[[noreturn]] void resource_fail(const char *message);
} // namespace xv::process::detail
