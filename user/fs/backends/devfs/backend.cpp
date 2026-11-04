module;
#include "kernel/support/types.h"
#include "kernel/io/fs_resources.h"
#include "user/runtime/user.h"
#include "kernel/fs/stat.h"
#include "kernel/fs/format/layout.h"
module xv.userfs.devfs;
import xv.userfs;
import xv.userfs.resources;

namespace userfs
{
bool DeviceBackend::decimal(const char *name, unsigned &number)
{
  number = 0;
  if (!name[0] || (name[0] == '0' && name[1])) return false;
  for (unsigned i = 0; name[i]; ++i)
  {
    if (i >= 2 || name[i] < '0' || name[i] > '9') return false;
    number = number * 10 + name[i] - '0';
    if (number >= 64) return false;
  }
  return true;
}

void DeviceBackend::name(unsigned number, char *out)
{
  if (number >= 10) *out++ = '0' + number / 10;
  *out++ = '0' + number % 10;
  *out = 0;
}

bool DeviceBackend::device(unsigned index, Attributes &out, char *name)
{
  xv::fsservice::Device record{};
  if (resources_.query(xv::fsservice::Operation::Device, index, record) != 1)
    return false;
  out = {DeviceBase + index, 0, 0, 0, record.mode,
         record.block ? unsigned(T_BLOCK) : unsigned(T_DEVICE), 1,
         record.major, record.minor};
  if (name) strcpy(name, record.name);
  return true;
}

bool DeviceBackend::pty(unsigned number, Attributes &out, bool lookup)
{
  xv::fsservice::Pty record{};
  // Names require current publication; metadata on an already open node may
  // survive master hangup. Both share one aggregate pin until Forget/death.
  auto operation = lookup ? xv::fsservice::Operation::RetainPty
                          : xv::fsservice::Operation::InspectPty;
  if (resources_.query(operation, number, record) != 1)
    return false;
  out = {PtyBase + number, 0, record.uid, record.gid, 0600, T_DEVICE, 1, 5, number};
  // Readdir uses a non-retaining enumeration query below.
  return true;
}

int DeviceBackend::getattr(unsigned node, Attributes &out)
{
  if (node == Root || node == Pts)
  {
    out = {node, 0, 0, 0, 0755, T_DIR, 2};
    return 0;
  }
  if (node >= DeviceBase && node < DeviceBase + 32)
    return device(node - DeviceBase, out) ? 0 : -1;
  if (node >= PtyBase && node < PtyBase + 64)
    return pty(node - PtyBase, out, false) ? 0 : -1;
  return -1;
}

int DeviceBackend::lookup(unsigned parent, const char *entry, Attributes &out)
{
  if (parent != Root && parent != Pts) return -1;
  if (!strcmp(entry, ".")) return getattr(parent, out);
  if (!strcmp(entry, "..")) return getattr(Root, out);
  if (parent == Root)
  {
    if (!strcmp(entry, "pts")) return getattr(Pts, out);
    for (unsigned index = 0; index < 32; ++index)
    {
      char name[32];
      if (!device(index, out, name)) return -1;
      if (!strcmp(entry, name)) return 0;
    }
    return -1;
  }
  unsigned number;
  return decimal(entry, number) && pty(number, out, true) ? 0 : -1;
}

int DeviceBackend::readdir(unsigned parent, unsigned long offset, Entry &out)
{
  if (parent != Root && parent != Pts) return -1;
  if (offset < 2)
  {
    strcpy(out.name, offset ? ".." : ".");
    if (getattr(offset ? Root : parent, out.attributes) < 0) return -1;
  }
  else if (parent == Root)
  {
    if (offset == 2)
    {
      strcpy(out.name, "pts");
      if (getattr(Pts, out.attributes) < 0) return -1;
    }
    else if (offset >= 35 || !device(offset - 3, out.attributes, out.name)) return 0;
  }
  else
  {
    if (offset >= 66) return 0;
    xv::fsservice::Pty record{};
    int result = resources_.query(xv::fsservice::Operation::Pty, offset - 2, record);
    if (result != 1) return result;
    name(record.number, out.name);
    out.attributes = {PtyBase + record.number, 0, record.uid, record.gid,
                      0600, T_DEVICE, 1, 5, record.number};
  }
  out.next = offset + 1;
  return 1;
}

int DeviceBackend::read(unsigned node, unsigned long offset, void *data, unsigned count)
{
  if (node != Root && node != Pts) return -1;
  unsigned done = 0;
  while (done < count)
  {
    Entry entry;
    int result = readdir(node, (offset + done) / sizeof(dirent), entry);
    if (result < 0) return done ? int(done) : -1;
    if (!result) break;
    dirent record{};
    record.inum = entry.attributes.node;
    for (unsigned i = 0; i < DIRSIZ && entry.name[i]; ++i) record.name[i] = entry.name[i];
    unsigned begin = (offset + done) % sizeof(record);
    unsigned take = sizeof(record) - begin;
    if (take > count - done) take = count - done;
    memmove(static_cast<unsigned char *>(data) + done,
            reinterpret_cast<unsigned char *>(&record) + begin, take);
    done += take;
  }
  return done;
}

void DeviceBackend::forget(unsigned node)
{
  if (node >= PtyBase && node < PtyBase + 64)
  {
    xv::fsservice::Pty ignored{};
    (void)resources_.query(xv::fsservice::Operation::ReleasePty, node - PtyBase, ignored);
  }
}
} // namespace userfs
