module;
#include "user/runtime/user.h"
#include "user/fs/supervisor/workspace.h"
#include "kernel/fs/fcntl.h"
#include "kernel/io/boot_assets.h"
#include "kernel/io/dirent.h"
module xv.userfs.catalog;
namespace userfs::catalog
{
namespace
{
bool copy(char *out, unsigned capacity, const char *value)
{
  unsigned n = strlen(value);
  if (n >= capacity) return false;
  memmove(out, value, n + 1);
  return true;
}
bool identifier(const char *text)
{
  if (!*text) return false;
  for (unsigned i = 0; text[i]; ++i)
  {
    char c = text[i];
    if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.')) return false;
  }
  return true;
}
bool read_manifest(int fd, Plugin &plugin)
{
  struct Text { char bytes[2049]; };
  detail::Workspace<Text> storage;
  if (!storage) { close(fd); return false; }
  auto &text = storage->bytes;
  int used = 0, count = 0;
  while (used < 2048 && (count = read(fd, text + used, 2048 - used)) > 0) used += count;
  char extra;
  bool ended = used < 2048 ? count == 0 : read(fd, &extra, 1) == 0;
  close(fd);
  return ended && parse(text, used, plugin);
}
bool suffix(const char *text, const char *ending)
{
  unsigned n = strlen(text), m = strlen(ending);
  return n >= m && !strcmp(text + n - m, ending);
}
}
bool mode(const char *text, unsigned &value)
{
  value = 0;
  if (!*text) return false;
  for (unsigned i = 0; text[i]; ++i)
  {
    if (i >= 5 || text[i] < '0' || text[i] > '7') return false;
    value = value * 8 + text[i] - '0';
  }
  return value <= 07777;
}
bool parse(const char *text, unsigned size, Plugin &out)
{
  if (!text || !size || size > 2048) return false;
  detail::Workspace<Plugin> allocation;
  if (!allocation) return false;
  auto &candidate = *allocation;
  unsigned seen = 0, offset = 0;
  while (offset < size)
  {
    char line[384];
    unsigned length = 0;
    while (offset < size && text[offset] != '\n')
    {
      if (length + 1 == sizeof(line) || !text[offset] || text[offset] == '\r') return false;
      line[length++] = text[offset++];
    }
    if (offset < size) ++offset;
    line[length] = 0;
    if (!length || line[0] == '#') continue;
    unsigned equal = 0;
    while (equal < length && line[equal] != '=') ++equal;
    if (!equal || equal == length) return false;
    line[equal] = 0;
    const char *value = line + equal + 1;
    unsigned bit = 0;
    bool valid = false;
    if (!strcmp(line, "abi")) { bit = 1; valid = !strcmp(value, "1"); }
    else if (!strcmp(line, "type")) { bit = 2; valid = identifier(value) && copy(candidate.type, sizeof(candidate.type), value); }
    else if (!strcmp(line, "exec")) { bit = 4; valid = *value == '/' && copy(candidate.executable, sizeof(candidate.executable), value); }
    else if (!strcmp(line, "image")) { bit = 8; valid = *value && copy(candidate.image, sizeof(candidate.image), value); }
    else if (!strcmp(line, "resource"))
    {
      bit = 16; valid = true;
      if (!strcmp(value, "none")) candidate.resource = Resource::None;
      else if (!strcmp(value, "block")) candidate.resource = Resource::Block;
      else if (!strcmp(value, "system")) candidate.resource = Resource::System;
      else if (!strcmp(value, "file")) candidate.resource = Resource::File;
      else valid = false;
    }
    else if (!strcmp(line, "alias")) { bit = 32; valid = identifier(value) && copy(candidate.alias, sizeof(candidate.alias), value); }
    else if (!strcmp(line, "source")) { bit = 64; valid = copy(candidate.source, sizeof(candidate.source), value); }
    else if (!strcmp(line, "options")) { bit = 128; valid = copy(candidate.options, sizeof(candidate.options), value); }
    else if (!strcmp(line, "startup_ms"))
    {
      bit = 2048;
      unsigned long number = 0;
      valid = *value && strlen(value) <= 10;
      for (unsigned i = 0; valid && value[i]; ++i)
      {
        valid = value[i] >= '0' && value[i] <= '9';
        number = number * 10 + value[i] - '0';
      }
      valid = valid && number > 0 && number <= 0xffffffffUL;
      candidate.startup_ms = number;
    }
    else if (!strcmp(line, "access"))
    { bit = 512; valid = !strcmp(value, "ro") || !strcmp(value, "rw"); candidate.writable = !strcmp(value, "rw"); }
    else if (!strcmp(line, "system"))
    { bit = 1024; valid = !strcmp(value, "0") || !strcmp(value, "1"); candidate.system = !strcmp(value, "1"); }
    else if (!strcmp(line, "mode")) { bit = 256; valid = mode(value, candidate.mode); }
    if (!valid || (seen & bit)) return false;
    seen |= bit;
  }
  if ((seen & 23) != 23 || (candidate.alias[0] && !strcmp(candidate.type, candidate.alias))) return false;
  out = candidate;
  return true;
}
int asset(const char *name)
{
  xv::bootdata::Request request;
  if (!copy(request.name, sizeof(request.name), name)) return -1;
  return ioctl(4, xv::bootdata::OpenIoctl, reinterpret_cast<unsigned long>(&request));
}
bool bundled(unsigned index, Plugin &out)
{
  for (unsigned at = 0, selected = 0;; ++at)
  {
    xv::bootdata::Request request;
    request.index = at;
    int fd = ioctl(4, xv::bootdata::OpenIoctl, reinterpret_cast<unsigned long>(&request));
    if (fd < 0) return false;
    if ((strlen(request.name) < 10 || memcmp(request.name, "fsplugins/", 10)) || !suffix(request.name, ".conf")) { close(fd); continue; }
    Plugin candidate;
    if (!read_manifest(fd, candidate)) continue;
    if (selected++ != index) continue;
    out = candidate;
    return true;
  }
}
bool installed(const char *type, Plugin &out)
{
  int directory = open("/etc/fsplugins", O_RDONLY);
  if (directory < 0) return false;
  bool found = false, valid = true;
  struct Scan { alignas(8) unsigned char entries[1024]; Plugin candidate; };
  detail::Workspace<Scan> scan;
  if (!scan) { close(directory); return false; }
  auto &entries = scan->entries;
  int count;
  while (valid && (count = getdents64(directory, entries, sizeof(entries))) > 0)
  {
    for (unsigned offset = 0; offset < unsigned(count);)
    {
      auto *entry = reinterpret_cast<linux_dirent64 *>(entries + offset);
      if (entry->d_reclen < DIRENT64_NAME_OFFSET + 1 || entry->d_reclen > unsigned(count) - offset)
      { valid = false; break; }
      const char *name = entry->d_name;
      unsigned limit = entry->d_reclen - DIRENT64_NAME_OFFSET, length = 0;
      while (length < limit && name[length]) ++length;
      if (length == limit) { valid = false; break; }
      offset += entry->d_reclen;
      if (length >= 128 - 15 || !suffix(name, ".conf")) continue;
      char path[128];
      strcpy(path, "/etc/fsplugins/");
      strcpy(path + 15, name);
      int fd = open(path, O_RDONLY);
      auto &candidate = scan->candidate;
      if (fd < 0 || !read_manifest(fd, candidate)) continue;
      if (strcmp(candidate.type, type) && strcmp(candidate.alias, type)) continue;
      if (found) { valid = false; break; }
      out = candidate; found = true;
    }
  }
  close(directory);
  return valid && count == 0 && found;
}
}
