#include "file.h"
#include "kernel/fs/fcntl.h"
#include "user/runtime/directory.h"
#include "user/runtime/user.h"

namespace vim
{
namespace
{
constexpr uint64 hash_start = 14695981039346656037ULL;

uint64
hash_bytes(uint64 hash, const char *data, unsigned size) {
  for (unsigned i = 0; i < size; ++i)
  {
    hash = (hash ^ static_cast<unsigned char>(data[i])) * 1099511628211ULL;
  }
  return hash;
}

bool
regular(const struct stat &st) {
  return st.type == T_FILE && st.nlink == 1;
}

enum class Presence
{
  Missing,
  Present,
  Unavailable
};

Presence
inspect(const char *path, char *workspace) {
  struct stat metadata{};
  if (lstat(path, &metadata) == 0)
  {
    return Presence::Present;
  }
  // Legacy open/lstat return -1 without setting errno. Confirm absence through
  // the parent directory instead of interpreting a stale ENOENT value.
  strcpy(workspace, path);
  const char *name = path;
  char *last = nullptr;
  for (char *p = workspace; *p; ++p)
  {
    if (*p == '/')
    {
      last = p;
    }
  }
  if (last)
  {
    name = path + (last - workspace) + 1;
    if (last == workspace)
    {
      last[1] = 0;
    }
    else
    {
      *last = 0;
    }
  }
  else
  {
    strcpy(workspace, ".");
  }
  if (!*name)
  {
    return Presence::Unavailable;
  }
  int parent = open(workspace, O_RDONLY);
  if (parent < 0)
  {
    return Presence::Unavailable;
  }
  DirectoryReader reader(parent);
  const linux_dirent64 *entry;
  int result;
  while ((result = reader.next(entry)) == 1)
  {
    if (!strcmp(entry->d_name, name))
    {
      close(parent);
      return Presence::Unavailable;
    }
  }
  close(parent);
  return result == 0 ? Presence::Missing : Presence::Unavailable;
}

char *
decimal(char *out, unsigned value) {
  char digits[12];
  unsigned n = 0;
  do
  {
    digits[n++] = '0' + value % 10;
    value /= 10;
  } while (value);
  while (n)
  {
    *out++ = digits[--n];
  }
  *out = 0;
  return out;
}
} // namespace

bool
File::load(Buffer &buffer, const char *name) {
  if (!*name || strlen(name) >= sizeof(path))
  {
    error = "Invalid or too long filename";
    return false;
  }
  strcpy(path, name);
  int fd = open(path, O_RDONLY | O_NOFOLLOW);
  if (fd < 0)
  {
    if (inspect(path, temporary_) == Presence::Missing)
    {
      hash_ = hash_start;
      buffer.saved();
      return true;
    }
    error = "Cannot open file (symlinks are not supported)";
    return false;
  }
  if (fstat(fd, &original_) < 0 || !regular(original_) ||
      original_.size > Buffer::limit)
  {
    close(fd);
    error = "Requires a regular, singly linked file of at most 1 MiB";
    return false;
  }
  char bytes[512];
  int count;
  uint64 hash = hash_start;
  while ((count = read(fd, bytes, sizeof(bytes))) > 0)
  {
    for (int i = 0; i < count; ++i)
    {
      if (!bytes[i])
      {
        close(fd);
        error = "Binary files containing NUL are unsupported";
        return false;
      }
    }
    if (!buffer.insert(buffer.size(), bytes, count))
    {
      close(fd);
      error = "File too large or out of memory";
      return false;
    }
    hash = hash_bytes(hash, bytes, count);
  }
  close(fd);
  if (count < 0)
  {
    error = "File read failed";
    return false;
  }
  exists_ = true;
  hash_ = hash;
  buffer.saved();
  return true;
}

bool
File::save(Buffer &buffer, const char *name) {
  error = nullptr;
  const char *target = name && *name ? name : path;
  if (!*target || strlen(target) + 32 >= MAXPATH)
  {
    error = "Filename missing or too long for temporary save";
    return false;
  }
  bool same = !strcmp(target, path);
  struct stat st{};
  int current = open(target, O_RDONLY | O_NOFOLLOW);
  bool present = current >= 0;
  if (!present && inspect(target, temporary_) != Presence::Missing)
  {
    error = "Cannot inspect destination";
    return false;
  }
  if (present)
  {
    bool okay = fstat(current, &st) == 0 && regular(st) && same && exists_ &&
                st.dev == original_.dev && st.ino == original_.ino &&
                access(target, W_OK) == 0;
    uint64 hash = hash_start;
    unsigned size = 0;
    int count = 0;
    char bytes[512];
    while (okay && (count = read(current, bytes, sizeof(bytes))) > 0)
    {
      if (size + count > Buffer::limit)
      {
        okay = false;
        break;
      }
      size += count;
      hash = hash_bytes(hash, bytes, count);
    }
    close(current);
    if (!okay || count < 0 || hash != hash_)
    {
      error =
          "Destination changed, exists under another name, is linked or is not writable";
      return false;
    }
  }
  else if (same && exists_)
  {
    error = "Original file disappeared; use :w another-name";
    return false;
  }
  char *temporary = temporary_;
  strcpy(temporary, target);
  char *end = temporary + strlen(temporary);
  strcpy(end, ".vim-");
  end += 5;
  end = decimal(end, getpid());
  *end++ = '-';
  int fd = -1;
  for (unsigned attempt = 0; attempt < 32; ++attempt)
  {
    decimal(end, attempt);
    unsigned mask = umask(0077);
    fd = open(temporary, O_WRONLY | O_CREATE | O_EXCL);
    umask(mask);
    struct stat collision{};
    if (fd >= 0 || lstat(temporary, &collision) < 0)
    {
      break;
    }
  }
  if (fd < 0)
  {
    error = "Cannot create temporary file; original kept";
    return false;
  }
  bool okay = true;
  if (present)
  {
    struct stat temp{};
    okay = fstat(fd, &temp) == 0;
    if (okay && (temp.uid != st.uid || temp.gid != st.gid))
    {
      okay = chown(temporary, st.uid, st.gid) == 0;
    }
  }
  unsigned mask = umask(0);
  umask(mask);
  if (okay)
  {
    okay = chmod(temporary, present ? st.mode & 0777 : 0666 & ~mask) == 0;
  }
  unsigned sent = 0;
  while (okay && sent < buffer.size())
  {
    int n = write(fd, buffer.data() + sent, buffer.size() - sent);
    if (n < 0 && errno == EINTR)
    {
      continue;
    }
    if (n <= 0)
    {
      okay = false;
    }
    else
    {
      sent += n;
    }
  }
  if (okay)
  {
    okay = fsync(fd) == 0;
  }
  if (close(fd) < 0)
  {
    okay = false;
  }
  if (okay)
  {
    okay = rename(temporary, target) == 0;
  }
  if (!okay)
  {
    unlink(temporary);
    error = "Save failed; original kept";
    return false;
  }
  // target may point inside path; update it only after publication.
  if (!same)
  {
    strcpy(path, target);
  }
  if (stat(path, &original_) < 0)
  {
    error = "Saved, but cannot inspect new file";
    return false;
  }
  exists_ = true;
  hash_ = hash_bytes(hash_start, buffer.data(), buffer.size());
  buffer.saved();
  return true;
}
} // namespace vim
