export module xv.fs;
export import :cache;
export import :inode;
export import :filesystem;
export import :mapping;
export import :vfs;
export import :file;

export namespace xv::fs
{
// File graph reclamation may close inodes and issue FS RPCs.
bool can_reclaim_file_objects();
}
