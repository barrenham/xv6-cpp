#include "user/fs/backends/xv6/volume.h"
#include "kernel/fs/stat.h"

namespace userfs::xv6
{
namespace
{
using xv::ext3::copy;
using xv::ext3::zero;
using xv::ext3::le16;
using xv::ext3::le32;
using xv::ext3::put16;
using xv::ext3::put32;
class Page
{
  Io &io_;
  unsigned char *bytes_;
public:
  explicit Page(Io &io) : io_(io), bytes_(static_cast<unsigned char *>(io.allocate_page())) {}
  ~Page() { if (bytes_) io_.free_page(bytes_); }
  unsigned char *get() { return bytes_; }
};
bool kind(unsigned type)
{
  return type == T_DIR || type == T_FILE || type == T_SYMLINK || type == T_DEVICE || type == T_BLOCK || type == T_SOCKET;
}
}
struct Volume::Transaction
{
  Volume &volume;
  bool active;
  explicit Transaction(Volume &volume) : volume(volume), active(volume.available() && !volume.count_) {}
  ~Transaction() { volume.discard(); }
  bool finish() { return active && volume.commit(); }
};
Volume::~Volume() { discard(); }
bool Volume::available()
{
  error_ = Error::None;
  return ready_ && !failed_ ? true : fail(Error::Io);
}
void Volume::discard()
{
  while (count_)
  {
    auto &entry = dirty_[--count_];
    io_.free_page(entry.bytes); entry = {};
  }
}
bool Volume::data_block(unsigned block) const
{
  return block >= first_data_ && block < layout_.size;
}
bool Volume::read_block(unsigned block, void *data)
{
  if (block >= layout_.size) return fail(Error::Corrupt);
  for (unsigned i = 0; i < count_; ++i)
    if (dirty_[i].block == block) { copy(data, dirty_[i].bytes, BSIZE); return true; }
  return io_.read(static_cast<unsigned long>(block) * BSIZE, data, BSIZE) || fail(Error::Io);
}
unsigned char *Volume::edit(unsigned block, bool fresh)
{
  if (block < layout_.inodestart || block >= layout_.size) { fail(Error::Corrupt); return nullptr; }
  for (unsigned i = 0; i < count_; ++i) if (dirty_[i].block == block) return dirty_[i].bytes;
  if (count_ >= layout_.nlog - 1 || count_ == LOGBLOCKS) { fail(Error::NoSpace); return nullptr; }
  auto *data = static_cast<unsigned char *>(io_.allocate_page());
  if (!data) { fail(Error::NoMemory); return nullptr; }
  if (fresh) zero(data, BSIZE);
  else if (!read_block(block, data)) { io_.free_page(data); return nullptr; }
  dirty_[count_++] = {block, data};
  return data;
}
bool Volume::commit()
{
  if (!count_) return true;
  Page header(io_);
  if (!header.get()) return fail(Error::NoMemory);
  zero(header.get(), BSIZE);
  put32(header.get(), count_);
  for (unsigned i = 0; i < count_; ++i) put32(header.get() + 4 + i * 4, dirty_[i].block);
  auto write = [&](unsigned block, const void *data)
  { return io_.write(static_cast<unsigned long>(block) * BSIZE, data, BSIZE); };
  bool okay = true;
  for (unsigned i = 0; i < count_ && okay; ++i) okay = write(layout_.logstart + 1 + i, dirty_[i].bytes);
  // The header is published only after every redo block is durable. Home
  // installation and header clearing each have their own flush boundary.
  okay = okay && io_.flush() && write(layout_.logstart, header.get()) && io_.flush();
  for (unsigned i = 0; i < count_ && okay; ++i) okay = write(dirty_[i].block, dirty_[i].bytes);
  okay = okay && io_.flush();
  if (okay)
  {
    zero(header.get(), BSIZE);
    okay = write(layout_.logstart, header.get()) && io_.flush();
  }
  if (!okay) { failed_ = true; return fail(Error::Io); }
  discard();
  return true;
}
bool Volume::recover()
{
  Page header(io_), data(io_);
  if (!header.get() || !data.get()) return fail(Error::NoMemory);
  if (!read_block(layout_.logstart, header.get())) return false;
  unsigned count = le32(header.get());
  if (count > layout_.nlog - 1 || count > LOGBLOCKS) return fail(Error::Corrupt);
  for (unsigned i = 0; i < count; ++i)
  {
    unsigned block = le32(header.get() + 4 + i * 4);
    if (block < layout_.inodestart || block >= layout_.size) return fail(Error::Corrupt);
    for (unsigned j = 0; j < i; ++j)
      if (block == le32(header.get() + 4 + j * 4)) return fail(Error::Corrupt);
  }
  if (!count) return true;
  for (unsigned i = 0; i < count; ++i)
    if (!read_block(layout_.logstart + 1 + i, data.get()) ||
        !io_.write(static_cast<unsigned long>(le32(header.get() + 4 + i * 4)) * BSIZE,
                   data.get(), BSIZE)) return fail(Error::Io);
  if (!io_.flush()) return fail(Error::Io);
  zero(header.get(), BSIZE);
  return (io_.write(static_cast<unsigned long>(layout_.logstart) * BSIZE, header.get(), BSIZE) &&
          io_.flush()) || fail(Error::Io);
}
bool Volume::mount()
{
  discard(); ready_ = failed_ = false; error_ = Error::None;
  Page page(io_);
  if (!page.get()) return fail(Error::NoMemory);
  if (!io_.read(BSIZE, page.get(), BSIZE)) return fail(Error::Io);
  auto *raw = page.get();
  layout_ = {le32(raw), le32(raw + 4), le32(raw + 8), le32(raw + 12),
             le32(raw + 16), le32(raw + 20), le32(raw + 24), le32(raw + 28)};
  unsigned long inode_end = static_cast<unsigned long>(layout_.inodestart) +
                           (static_cast<unsigned long>(layout_.ninodes) + IPB - 1) / IPB;
  unsigned long data_start = static_cast<unsigned long>(layout_.bmapstart) +
                            (static_cast<unsigned long>(layout_.size) + BPB - 1) / BPB;
  // A bounded bitmap footprint ensures full-file truncation fits the fixed
  // journal, rather than partially publishing a multi-transaction truncation.
  if (layout_.magic != FSMAGIC || layout_.size > io_.size_bytes() / BSIZE ||
      layout_.ninodes < 2 || layout_.ninodes > 65536 || layout_.logstart < 2 ||
      layout_.nlog < MAXOPBLOCKS + 1 || layout_.nlog > LOGBLOCKS + 1 ||
      static_cast<unsigned long>(layout_.logstart) + layout_.nlog > layout_.inodestart ||
      inode_end > layout_.bmapstart || data_start >= layout_.size ||
      !layout_.nblocks || layout_.nblocks >= layout_.size ||
      layout_.size - layout_.nblocks < data_start ||
      (static_cast<unsigned long>(layout_.size) + BPB - 1) / BPB + 3 > layout_.nlog - 1)
    return fail(Error::Corrupt);
  first_data_ = layout_.size - layout_.nblocks;
  if (!recover()) return false;
  ready_ = true;
  Inode root;
  if (!load(ROOTINO, root) || root.value.type != T_DIR) { ready_ = false; return fail(Error::Corrupt); }
  // xv6 uses allocated zero-link inodes as its durable orphan set.
  for (unsigned number = 2; number < layout_.ninodes; ++number)
  {
    Inode inode;
    if (!load(number, inode, false)) { ready_ = false; return false; }
    if (inode.value.type && !inode.value.nlink && !reclaim(number)) { ready_ = false; return false; }
  }
  return true;
}
bool Volume::load(unsigned number, Inode &out, bool allocated)
{
  if (!number || number >= layout_.ninodes) return fail(Error::NotFound);
  Page page(io_);
  if (!page.get()) return fail(Error::NoMemory);
  if (!read_block(IBLOCK(number, layout_), page.get())) return false;
  const auto *raw = page.get() + (number % IPB) * sizeof(dinode);
  out = {}; out.number = number;
  auto &node = out.value;
  node.type = le16(raw); node.major = le16(raw + 2); node.minor = le16(raw + 4);
  node.nlink = le16(raw + 6); node.size = le32(raw + 8);
  for (unsigned i = 0; i <= NDIRECT; ++i) node.addrs[i] = le32(raw + 12 + i * 4);
  node.uid = le32(raw + 64); node.gid = le32(raw + 68); node.mode = le16(raw + 72);
  if (!node.type) return !allocated || fail(Error::NotFound);
  if (!kind(node.type) || node.nlink < 0 || node.size > MAXFILE * BSIZE || node.mode > 07777 ||
      node.major < 0 || node.minor < 0 || (node.type == T_DIR && node.size % sizeof(dirent)))
    return fail(Error::Corrupt);
  return true;
}
bool Volume::store(const Inode &inode)
{
  auto *data = edit(IBLOCK(inode.number, layout_));
  if (!data) return false;
  auto *raw = data + (inode.number % IPB) * sizeof(dinode);
  zero(raw, sizeof(dinode));
  const auto &node = inode.value;
  put16(raw, node.type); put16(raw + 2, node.major); put16(raw + 4, node.minor);
  put16(raw + 6, node.nlink); put32(raw + 8, node.size);
  for (unsigned i = 0; i <= NDIRECT; ++i) put32(raw + 12 + i * 4, node.addrs[i]);
  put32(raw + 64, node.uid); put32(raw + 68, node.gid); put16(raw + 72, node.mode);
  return true;
}
unsigned Volume::allocate_block()
{
  Page page(io_);
  if (!page.get()) { fail(Error::NoMemory); return 0; }
  for (unsigned base = 0; base < layout_.size; base += BPB)
  {
    if (!read_block(BBLOCK(base, layout_), page.get())) return 0;
    for (unsigned bit = 0; bit < BPB && bit < layout_.size - base; ++bit)
    {
      unsigned number = base + bit;
      if (!data_block(number) || (page.get()[bit / 8] & (1U << (bit % 8)))) continue;
      auto *bitmap = edit(BBLOCK(number, layout_));
      if (!bitmap) return 0;
      bitmap[bit / 8] |= 1U << (bit % 8);
      if (!edit(number, true)) return 0;
      return number;
    }
  }
  fail(Error::NoSpace); return 0;
}
bool Volume::free_block(unsigned number)
{
  if (!data_block(number)) return fail(Error::Corrupt);
  auto *bitmap = edit(BBLOCK(number, layout_));
  if (!bitmap) return false;
  unsigned bit = number % BPB;
  if (!(bitmap[bit / 8] & (1U << (bit % 8)))) return fail(Error::Corrupt);
  bitmap[bit / 8] &= ~(1U << (bit % 8));
  return true;
}
unsigned Volume::map(Inode &inode, unsigned index, bool create)
{
  if (index >= MAXFILE) { fail(Error::Invalid); return 0; }
  auto &node = inode.value;
  if (index < NDIRECT)
  {
    if (!node.addrs[index] && create) node.addrs[index] = allocate_block();
    if (node.addrs[index] && !data_block(node.addrs[index])) { fail(Error::Corrupt); return 0; }
    return node.addrs[index];
  }
  if (!node.addrs[NDIRECT] && create) node.addrs[NDIRECT] = allocate_block();
  if (!node.addrs[NDIRECT]) return 0;
  if (!data_block(node.addrs[NDIRECT])) { fail(Error::Corrupt); return 0; }
  unsigned number;
  if (create)
  {
    auto *addresses = edit(node.addrs[NDIRECT]);
    if (!addresses) return 0;
    number = le32(addresses + (index - NDIRECT) * 4);
    if (!number)
    {
      number = allocate_block();
      if (!number) return 0;
      put32(addresses + (index - NDIRECT) * 4, number);
    }
  }
  else
  {
    Page page(io_);
    if (!page.get()) { fail(Error::NoMemory); return 0; }
    if (!read_block(node.addrs[NDIRECT], page.get())) return 0;
    number = le32(page.get() + (index - NDIRECT) * 4);
  }
  if (number && !data_block(number)) { fail(Error::Corrupt); return 0; }
  return number;
}
int Volume::read_raw(Inode &inode, unsigned long offset, void *data, unsigned count)
{
  if (offset >= inode.value.size) return 0;
  if (count > inode.value.size - offset) count = inode.value.size - offset;
  Page page(io_);
  if (!page.get()) { fail(Error::NoMemory); return -1; }
  unsigned done = 0;
  while (done < count)
  {
    unsigned number = map(inode, (offset + done) / BSIZE, false);
    if (!number && error_ != Error::None) return -1;
    unsigned start = (offset + done) % BSIZE, take = BSIZE - start;
    if (take > count - done) take = count - done;
    if (number)
    {
      if (!read_block(number, page.get())) return -1;
      copy(static_cast<unsigned char *>(data) + done, page.get() + start, take);
    }
    else zero(static_cast<unsigned char *>(data) + done, take);
    done += take;
  }
  return done;
}
bool Volume::write_raw(Inode &inode, unsigned offset, const void *data, unsigned count)
{
  if (offset > MAXFILE * BSIZE || count > MAXFILE * BSIZE - offset) return fail(Error::NoSpace);
  unsigned done = 0;
  while (done < count)
  {
    unsigned number = map(inode, (offset + done) / BSIZE, true);
    if (!number) return false;
    auto *block = edit(number);
    if (!block) return false;
    unsigned start = (offset + done) % BSIZE, take = BSIZE - start;
    if (take > count - done) take = count - done;
    copy(block + start, static_cast<const unsigned char *>(data) + done, take);
    done += take;
  }
  if (count && offset + count > inode.value.size) inode.value.size = offset + count;
  return store(inode);
}
bool Volume::resize_raw(Inode &inode, unsigned size)
{
  if (size > MAXFILE * BSIZE) return fail(Error::NoSpace);
  auto &node = inode.value;
  if (size < node.size)
  {
    unsigned keep = (size + BSIZE - 1) / BSIZE;
    for (unsigned i = keep; i < NDIRECT; ++i)
      if (node.addrs[i]) { if (!free_block(node.addrs[i])) return false; node.addrs[i] = 0; }
    if (node.addrs[NDIRECT])
    {
      if (!data_block(node.addrs[NDIRECT])) return fail(Error::Corrupt);
      auto *addresses = edit(node.addrs[NDIRECT]);
      if (!addresses) return false;
      bool remaining = false;
      for (unsigned i = 0; i < NINDIRECT; ++i)
      {
        unsigned number = le32(addresses + i * 4);
        if (number && i + NDIRECT >= keep)
        {
          if (!free_block(number)) return false;
          put32(addresses + i * 4, 0);
        }
        else if (number) remaining = true;
      }
      if (!remaining)
      { if (!free_block(node.addrs[NDIRECT])) return false; node.addrs[NDIRECT] = 0; }
    }
    if (size % BSIZE)
    {
      unsigned number = map(inode, size / BSIZE, false);
      if (!number && error_ != Error::None) return false;
      if (number)
      {
        auto *data = edit(number);
        if (!data) return false;
        zero(data + size % BSIZE, BSIZE - size % BSIZE);
      }
    }
  }
  node.size = size;
  return store(inode);
}
bool Volume::read_inode(unsigned number, Inode &out)
{
  return available() && load(number, out);
}
int Volume::read(unsigned number, unsigned long offset, void *data, unsigned count)
{
  Inode inode;
  if (!available() || !load(number, inode)) return -1;
  return read_raw(inode, offset, data, count);
}
int Volume::write(unsigned number, unsigned long offset, const void *data, unsigned count)
{
  Transaction transaction(*this); Inode inode;
  if (!transaction.active || !load(number, inode) || offset > ~0U ||
      (inode.value.type != T_FILE && inode.value.type != T_SYMLINK) ||
      !write_raw(inode, offset, data, count) || !transaction.finish()) return -1;
  return count;
}
bool Volume::resize(unsigned number, unsigned size)
{
  Transaction transaction(*this); Inode inode;
  return transaction.active && load(number, inode) &&
         (inode.value.type == T_FILE || inode.value.type == T_SYMLINK) &&
         resize_raw(inode, size) && transaction.finish();
}
bool Volume::setattr(unsigned number, unsigned uid, unsigned gid, unsigned mode)
{
  Transaction transaction(*this); Inode inode;
  if (!transaction.active || mode > 07777 || !load(number, inode)) return false;
  inode.value.uid = uid; inode.value.gid = gid; inode.value.mode = mode;
  return store(inode) && transaction.finish();
}
bool Volume::allocate_inode(unsigned type, unsigned mode, unsigned uid, unsigned gid,
                             unsigned major, unsigned minor, Inode &out)
{
  if (!kind(type) || mode > 07777 || major > 32767 || minor > 32767) return fail(Error::Invalid);
  for (unsigned number = 2; number < layout_.ninodes; ++number)
  {
    Inode inode;
    if (!load(number, inode, false)) return false;
    if (inode.value.type) continue;
    out = {}; out.number = number;
    out.value.type = type; out.value.nlink = 1; out.value.uid = uid; out.value.gid = gid;
    out.value.mode = mode; out.value.major = major; out.value.minor = minor;
    return store(out);
  }
  return fail(Error::NoSpace);
}
bool Volume::component(const char *name, bool special)
{
  if (!name || !*name || (!special && (equal_name(name, ".") || equal_name(name, "..")))) return false;
  for (unsigned i = 0; i <= DIRSIZ; ++i)
  { if (!name[i]) return true; if (i == DIRSIZ || name[i] == '/') return false; }
  return false;
}
bool Volume::equal_name(const char *disk, const char *name)
{
  for (unsigned i = 0; i < DIRSIZ; ++i)
  {
    if (disk[i] != name[i]) return false;
    if (!name[i]) return true;
  }
  return !name[DIRSIZ];
}
bool Volume::find_entry(Inode &directory, const char *name, unsigned &offset, unsigned &number)
{
  if (directory.value.type != T_DIR || !component(name, true)) return fail(Error::Invalid);
  dirent entry{};
  for (offset = 0; offset < directory.value.size; offset += sizeof(entry))
  {
    if (read_raw(directory, offset, &entry, sizeof(entry)) != sizeof(entry)) return false;
    if (!entry.inum || !equal_name(entry.name, name)) continue;
    if (entry.inum >= layout_.ninodes) return fail(Error::Corrupt);
    number = entry.inum; return true;
  }
  return fail(Error::NotFound);
}
bool Volume::lookup(unsigned parent, const char *name, Inode &out)
{
  Inode directory; unsigned offset = 0, number = 0;
  if (!available() || !load(parent, directory) || !find_entry(directory, name, offset, number)) return false;
  if (!load(number, out)) return error_ == Error::NotFound ? fail(Error::Corrupt) : false;
  return true;
}
bool Volume::add_entry(Inode &directory, const char *name, unsigned number)
{
  if (!component(name, true)) return fail(Error::Invalid);
  dirent entry{};
  unsigned offset = 0;
  for (; offset < directory.value.size; offset += sizeof(entry))
  {
    if (read_raw(directory, offset, &entry, sizeof(entry)) != sizeof(entry)) return false;
    if (!entry.inum) break;
  }
  entry = {}; entry.inum = number;
  for (unsigned i = 0; i < DIRSIZ && name[i]; ++i) entry.name[i] = name[i];
  return write_raw(directory, offset, &entry, sizeof(entry));
}
bool Volume::empty(Inode &directory)
{
  dirent entry{};
  for (unsigned offset = 0; offset < directory.value.size; offset += sizeof(entry))
  {
    if (read_raw(directory, offset, &entry, sizeof(entry)) != sizeof(entry)) return false;
    if (entry.inum && !equal_name(entry.name, ".") && !equal_name(entry.name, "..")) return fail(Error::NotEmpty);
  }
  return true;
}
bool Volume::create(unsigned parent, const char *name, unsigned type, unsigned mode,
                      unsigned uid, unsigned gid, unsigned major, unsigned minor, Inode &out)
{
  Transaction transaction(*this); Inode directory; unsigned offset = 0, number = 0;
  if (!transaction.active || !component(name) || !load(parent, directory) || !directory.value.nlink) return false;
  if (find_entry(directory, name, offset, number)) return fail(Error::Exists);
  if (error_ != Error::NotFound) return false;
  error_ = Error::None;
  if (type == T_DIR && directory.value.nlink == NLINK_MAX) return fail(Error::NoSpace);
  if (!allocate_inode(type, mode, uid, gid, major, minor, out)) return false;
  if (type == T_DIR)
  {
    if (!add_entry(out, ".", out.number) || !add_entry(out, "..", parent)) return false;
    ++directory.value.nlink;
  }
  return add_entry(directory, name, out.number) && transaction.finish();
}
bool Volume::link(unsigned parent, const char *name, unsigned target)
{
  Transaction transaction(*this); Inode directory, inode; unsigned offset = 0, number = 0;
  if (!transaction.active || !component(name) || !load(parent, directory) || !load(target, inode) ||
      !directory.value.nlink || !inode.value.nlink || inode.value.type == T_DIR || inode.value.nlink == NLINK_MAX)
    return false;
  if (find_entry(directory, name, offset, number)) return fail(Error::Exists);
  if (error_ != Error::NotFound) return false;
  error_ = Error::None;
  ++inode.value.nlink;
  return store(inode) && add_entry(directory, name, target) && transaction.finish();
}
bool Volume::unlink(unsigned parent, const char *name, Inode &removed)
{
  Transaction transaction(*this); Inode directory; unsigned offset = 0, number = 0;
  if (!transaction.active || !component(name) || !load(parent, directory) || !directory.value.nlink ||
      !find_entry(directory, name, offset, number) || !load(number, removed) || !removed.value.nlink) return false;
  if (removed.value.type == T_DIR)
  { if (!empty(removed)) return false; --directory.value.nlink; }
  dirent blank{};
  --removed.value.nlink;
  return write_raw(directory, offset, &blank, sizeof(blank)) && store(removed) && transaction.finish();
}
bool Volume::rename(unsigned from, const char *old_name, unsigned to, const char *new_name,
                      unsigned &replaced)
{
  replaced = 0;
  Transaction transaction(*this);
  Inode source_parent, destination_parent, source, target;
  unsigned old_offset = 0, new_offset = 0, number = 0;
  if (!transaction.active || !component(old_name) || !component(new_name) ||
      !load(from, source_parent) || !load(to, destination_parent) ||
      !source_parent.value.nlink || !destination_parent.value.nlink ||
      !find_entry(source_parent, old_name, old_offset, number) || !load(number, source)) return false;
  bool exists = find_entry(destination_parent, new_name, new_offset, number);
  if (exists)
  {
    if (!load(number, target)) return false;
    if (source.number == target.number) return transaction.finish();
    if ((source.value.type == T_DIR) != (target.value.type == T_DIR) ||
        (target.value.type == T_DIR && !empty(target))) return false;
  }
  else if (error_ != Error::NotFound) return false;
  error_ = Error::None;
  if (source.value.type == T_DIR)
  {
    if (from != to && !exists && destination_parent.value.nlink == NLINK_MAX) return fail(Error::NoSpace);
    Inode ancestor = destination_parent;
    unsigned visited = 0;
    while (ancestor.number != ROOTINO)
    {
      if (ancestor.number == source.number || ++visited > layout_.ninodes) return fail(Error::Invalid);
      unsigned offset = 0, parent = 0;
      if (!find_entry(ancestor, "..", offset, parent) || !load(parent, ancestor)) return false;
    }
  }
  if (source.number == from || source.number == to ||
      (exists && (target.number == from || target.number == to))) return fail(Error::Corrupt);
  // Keep one authoritative parent object when both names share a directory.
  Inode &destination = from == to ? source_parent : destination_parent;
  dirent entry{}; entry.inum = source.number;
  for (unsigned i = 0; i < DIRSIZ && new_name[i]; ++i) entry.name[i] = new_name[i];
  if (exists)
  {
    if (!write_raw(destination, new_offset, &entry, sizeof(entry))) return false;
    --target.value.nlink;
    if (target.value.type == T_DIR) --destination.value.nlink;
    if (!store(target)) return false;
    replaced = target.number;
  }
  else if (from == to)
  { if (!write_raw(source_parent, old_offset, &entry, sizeof(entry))) return false; }
  else if (!add_entry(destination, new_name, source.number)) return false;
  if (exists || from != to)
  {
    dirent blank{};
    if (!write_raw(source_parent, old_offset, &blank, sizeof(blank))) return false;
  }
  if (source.value.type == T_DIR && from != to)
  {
    dirent parent{}; parent.inum = to; parent.name[0] = '.'; parent.name[1] = '.';
    if (!write_raw(source, sizeof(dirent), &parent, sizeof(parent))) return false;
    --source_parent.value.nlink; ++destination.value.nlink;
  }
  return store(source_parent) && (from == to || store(destination)) && transaction.finish();
}
bool Volume::next_entry(unsigned number, unsigned long &offset, Entry &out)
{
  Inode directory;
  if (!available() || !load(number, directory) || directory.value.type != T_DIR || offset % sizeof(dirent))
    return false;
  dirent entry{};
  while (offset < directory.value.size)
  {
    if (read_raw(directory, offset, &entry, sizeof(entry)) != sizeof(entry)) return false;
    offset += sizeof(entry);
    if (!entry.inum) continue;
    if (entry.inum >= layout_.ninodes) return fail(Error::Corrupt);
    out = {}; out.inode = entry.inum;
    for (unsigned i = 0; i < DIRSIZ && entry.name[i]; ++i) out.name[i] = entry.name[i];
    return component(out.name, true) || fail(Error::Corrupt);
  }
  return false;
}
bool Volume::reclaim(unsigned number)
{
  Transaction transaction(*this); Inode inode;
  if (!transaction.active || number == ROOTINO || !load(number, inode) || inode.value.nlink) return false;
  if (!resize_raw(inode, 0)) return false;
  inode.value = {};
  return store(inode) && transaction.finish();
}
bool Volume::sync()
{
  return available() && !count_ && (io_.flush() || fail(Error::Io));
}
} // namespace userfs::xv6
