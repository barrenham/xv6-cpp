#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sys/mman.h>
#include <sys/wait.h>
#include "ext3_image.h"

#define stat xv6_stat // avoid clash with host struct stat
#include "kernel/support/types.h"
#include "kernel/fs/format/layout.h"
#include "kernel/fs/stat.h"
#include "kernel/support/param.h"

#define NINODES 200

// Disk layout:
// [ boot block | sb block | log | inode blocks | free bit map | data blocks ]

int nbitmap = FSSIZE / BPB + 1;
int ninodeblocks = NINODES / IPB + 1;
int nlog = LOGBLOCKS + 1; // Header followed by LOGBLOCKS data blocks.
int nmeta;   // Number of meta blocks (boot, sb, nlog, inode, bitmap)
int nblocks; // Number of data blocks

int fsfd = -1;
std::string temporary_image;
struct superblock sb;
char zeroes[BSIZE];
uint freeinode = 1;
uint freeblock;

void balloc(int);
void wsect(uint, void *);
void winode(uint, struct dinode *);
void rinode(uint inum, struct dinode *ip);
void rsect(uint sec, void *buf);
uint ialloc(ushort type);
void iappend(uint inum, void *p, int n);
void die(const char *);

// convert to riscv byte order
ushort
xshort(ushort x)
{
  ushort y;
  uchar *a = (uchar *)&y;
  a[0] = x;
  a[1] = x >> 8;
  return y;
}

uint
xint(uint x)
{
  uint y;
  uchar *a = (uchar *)&y;
  a[0] = x;
  a[1] = x >> 8;
  a[2] = x >> 16;
  a[3] = x >> 24;
  return y;
}

[[noreturn]] void error(const char *message)
{
  fprintf(stderr, "mkfs: %s\n", message);
  if (fsfd >= 0) close(fsfd);
  if (!temporary_image.empty()) unlink(temporary_image.c_str());
  exit(1);
}

struct ImageEntry
{
  std::string kind, path, source;
  ushort mode;
};

bool ext3_format = true;

void validate_path(const std::string &path)
{
  if (path.size() < 2 || path.size() >= (ext3_format ? 4096:MAXPATH) || path.front() != '/' || path.back() == '/' || path.find('\0') != std::string::npos)
    error("image path must be a canonical absolute path");
  size_t start = 1;
  while (start < path.size())
  {
    size_t end = path.find('/', start);
    if (end == std::string::npos) end = path.size();
    std::string component = path.substr(start, end - start);
    if (component.empty() || component == "." || component == ".." || component.size() > (ext3_format ? 255:DIRSIZ))
      error("invalid image path component");
    start = end + 1;
  }
}

std::vector<ImageEntry> read_manifest(const char *path)
{
  std::ifstream input(path);
  if (!input) error("cannot open image manifest");
  std::vector<ImageEntry> entries;
  std::map<std::string, bool> names;
  std::string line;
  while (std::getline(input, line))
  {
    if (line.empty() || line[0] == '#') continue;
    std::vector<std::string> fields;
    size_t start = 0;
    for (;;)
    {
      size_t end = line.find('\t', start);
      fields.push_back(line.substr(start, end == std::string::npos ? end : end - start));
      if (end == std::string::npos) break;
      start = end + 1;
    }
    if (fields.size() < 3 ||
        (fields[0] == "dir" ? fields.size() != 3 : fields[0] != "file" || fields.size() != 4))
      error("expected dir<TAB>mode<TAB>path or file<TAB>mode<TAB>path<TAB>source");
    if (fields[1].empty() || fields[1].size() > 5) error("invalid file mode");
    unsigned mode = 0;
    for (char digit : fields[1])
    {
      if (digit < '0' || digit > '7') error("file mode must be octal");
      mode = mode * 8 + digit - '0';
    }
    if (mode > 07777 || (fields[0] == "dir" && (mode & 06000))) error("unsupported file mode");
    validate_path(fields[2]);
    if (!names.emplace(fields[2], true).second) error("duplicate image path");
    if (fields[0] == "file" && fields[3].empty()) error("empty host source path");
    entries.push_back({fields[0], fields[2], fields.size() == 4 ? fields[3] : "", static_cast<ushort>(mode)});
  }
  if (!input.eof()) error("cannot read image manifest");
  return entries;
}

void directory_entry(uint parent, const char *name, uint number)
{
  struct dirent entry{};
  entry.inum = xshort(number);
  strncpy(entry.name, name, DIRSIZ);
  iappend(parent, &entry, sizeof(entry));
}

void ext3_command(const char *program, const char *first, const char *image)
{
  pid_t child = fork();
  if (child < 0) die("fork formatter");
  if (!child)
  {
    if (!strcmp(program, "mke2fs"))
    {
      execlp(program, program, first, "-F", "-t", "ext3", "-b", "1024", "-E", "root_owner=0:0", image, nullptr);
      execl("/usr/sbin/mke2fs", program, first, "-F", "-t", "ext3", "-b", "1024", "-E", "root_owner=0:0", image, nullptr);
    }
    else
    {
      execlp(program, program, first, image, nullptr);
      execl("/usr/sbin/e2fsck", program, first, image, nullptr);
    }
    _exit(127);
  }
  int status;
  if (waitpid(child, &status, 0) != child || !WIFEXITED(status) || WEXITSTATUS(status))
    error("ext3 formatter/checker failed; install e2fsprogs");
}

int make_ext3(const char *output, const std::vector<ImageEntry> &entries)
{
  constexpr uint64_t capacity = 64UL * 1024 * 1024;
  if (ftruncate(fsfd, capacity) < 0 || close(fsfd) < 0) die("size ext3 image");
  fsfd = -1;
  ext3_command("mke2fs", "-q", temporary_image.c_str());
  fsfd = open(temporary_image.c_str(), O_RDWR);
  if (fsfd < 0) die("open ext3 image");
  void *bytes = mmap(nullptr, capacity, PROT_READ | PROT_WRITE, MAP_SHARED, fsfd, 0);
  if (bytes == MAP_FAILED) die("map ext3 image");
  unsigned status = 0;
  void *image = xv_ext3_open(bytes, capacity, &status);
  if (!image) error("cannot mount formatted ext3 image");
  std::map<std::string, uint> nodes{{"/", 2}};
  for (const auto &entry : entries)
  {
    size_t slash = entry.path.rfind('/');
    std::string parent = slash == 0 ? "/" : entry.path.substr(0, slash);
    auto found = nodes.find(parent);
    if (found == nodes.end()) error("parent directory must precede its children in manifest");
    uint number = 0;
    if (xv_ext3_create(image, found->second, entry.path.c_str() + slash + 1,
                       entry.kind == "dir" ? T_DIR:T_FILE, 0, 0, entry.mode, &number) < 0)
      error("ext3 inode creation failed");
    nodes.emplace(entry.path, number);
    if (entry.kind == "dir") continue;
    int source = open(entry.source.c_str(), O_RDONLY);
    if (source < 0) die(entry.source.c_str());
    char buffer[65536];ssize_t count;uint64_t offset = 0;
    while ((count = read(source, buffer, sizeof(buffer))) > 0)
    {
      if (xv_ext3_write(image, number, offset, buffer, uint64_t(count)) < 0) error("ext3 file write failed");
      offset += uint64_t(count);
    }
    if (count < 0 || close(source) < 0) die("read/close host file");
  }
  xv_ext3_close(image);
  if (msync(bytes, capacity, MS_SYNC) < 0 || munmap(bytes, capacity) < 0 || fsync(fsfd) < 0 || close(fsfd) < 0)
    die("persist ext3 image");
  fsfd = -1;
  ext3_command("e2fsck", "-fn", temporary_image.c_str());
  if (rename(temporary_image.c_str(), output) < 0) die("publish ext3 image");
  temporary_image.clear();
  printf("ext3: installed %zu entries\n", entries.size());
  return 0;
}

int main(int argc, char *argv[])
{
  static_assert(sizeof(int) == 4, "Integers must be 4 bytes!");
  const char *manifest = nullptr;
  for (int index = 2; index + 1 < argc; index += 2)
  {
    if (!strcmp(argv[index], "--manifest") && !manifest) manifest = argv[index + 1];
    else if (!strcmp(argv[index], "--filesystem"))
    {
      if (strcmp(argv[index + 1], "ext3") && strcmp(argv[index + 1], "xv6")) error("filesystem must be ext3 or xv6");
      ext3_format = !strcmp(argv[index + 1], "ext3");
    }
    else error("unknown or repeated mkfs option");
  }
  if (!manifest || argc % 2) error("usage: mkfs fs.img --manifest fs-manifest.tsv [--filesystem ext3|xv6]");
  auto entries = read_manifest(manifest);
  assert((BSIZE % sizeof(struct dinode)) == 0);
  assert((BSIZE % sizeof(struct dirent)) == 0);

  std::string pattern = std::string(argv[1]) + ".tmp.XXXXXX";
  std::vector<char> temporary(pattern.begin(), pattern.end());
  temporary.push_back(0);
  fsfd = mkstemp(temporary.data());
  if (fsfd < 0) die(argv[1]);
  temporary_image = temporary.data();
  if (ext3_format) return make_ext3(argv[1], entries);

  nmeta = 2 + nlog + ninodeblocks + nbitmap;
  nblocks = FSSIZE - nmeta;
  sb.magic = xint(FSMAGIC);
  sb.size = xint(FSSIZE);
  sb.nblocks = xint(nblocks);
  sb.ninodes = xint(NINODES);
  sb.nlog = xint(nlog);
  sb.logstart = xint(2);
  sb.inodestart = xint(2 + nlog);
  sb.bmapstart = xint(2 + nlog + ninodeblocks);
  printf("nmeta %d blocks %d total %d\n", nmeta, nblocks, FSSIZE);
  freeblock = nmeta;
  for (int i = 0; i < FSSIZE; ++i) wsect(i, zeroes);
  char buffer[BSIZE]{};
  memmove(buffer, &sb, sizeof(sb));
  wsect(1, buffer);

  uint root = ialloc(T_DIR);
  assert(root == ROOTINO);
  directory_entry(root, ".", root);
  directory_entry(root, "..", root);
  std::map<std::string, uint> nodes{{"/", root}};
  for (const auto &entry : entries)
  {
    size_t slash = entry.path.rfind('/');
    std::string parent_path = slash == 0 ? "/" : entry.path.substr(0, slash);
    auto found = nodes.find(parent_path);
    if (found == nodes.end()) error("parent directory must precede its children in manifest");
    uint parent = found->second;
    struct dinode directory;
    rinode(parent, &directory);
    if (xshort(directory.type) != T_DIR) error("image parent is not a directory");
    uint number = ialloc(entry.kind == "dir" ? T_DIR : T_FILE);
    struct dinode node;
    rinode(number, &node);
    node.mode = xshort(entry.mode);
    winode(number, &node);
    directory_entry(parent, entry.path.c_str() + slash + 1, number);
    nodes.emplace(entry.path, number);
    if (entry.kind == "dir")
    {
      directory_entry(number, ".", number);
      directory_entry(number, "..", parent);
      directory.nlink = xshort(xshort(directory.nlink) + 1);
      struct dinode updated;
      rinode(parent, &updated);
      updated.nlink = directory.nlink;
      winode(parent, &updated);
    }
    else
    {
      int fd = open(entry.source.c_str(), O_RDONLY);
      if (fd < 0) die(entry.source.c_str());
      ssize_t count;
      while ((count = read(fd, buffer, sizeof(buffer))) > 0) iappend(number, buffer, count);
      if (count < 0) die("read host file");
      if (close(fd) < 0) die("close host file");
    }
  }
  balloc(freeblock);
  if (fsync(fsfd) < 0) die("fsync image");
  if (close(fsfd) < 0) die("close image");
  fsfd = -1;
  if (rename(temporary_image.c_str(), argv[1]) < 0) die("publish image");
  temporary_image.clear();
  printf("installed %zu entries\n", entries.size());
  return 0;
}

void
wsect(uint sec, void *buf)
{
  if (lseek(fsfd, sec * BSIZE, 0) != sec * BSIZE)
    die("lseek");
  if (write(fsfd, buf, BSIZE) != BSIZE)
    die("write");
}

void
winode(uint inum, struct dinode *ip)
{
  char buf[BSIZE];
  uint bn;
  struct dinode *dip;

  bn = IBLOCK(inum, sb);
  rsect(bn, buf);
  dip = ((struct dinode *)buf) + (inum % IPB);
  *dip = *ip;
  wsect(bn, buf);
}

void
rinode(uint inum, struct dinode *ip)
{
  char buf[BSIZE];
  uint bn;
  struct dinode *dip;

  bn = IBLOCK(inum, sb);
  rsect(bn, buf);
  dip = ((struct dinode *)buf) + (inum % IPB);
  *ip = *dip;
}

void
rsect(uint sec, void *buf)
{
  if (lseek(fsfd, sec * BSIZE, 0) != sec * BSIZE)
    die("lseek");
  if (read(fsfd, buf, BSIZE) != BSIZE)
    die("read");
}

uint
ialloc(ushort type)
{
  if (freeinode >= NINODES) error("out of image inodes");
  uint inum = freeinode++;
  struct dinode din;

  bzero(&din, sizeof(din));
  din.type = xshort(type);
  din.uid = din.gid = 0; din.mode = xshort(type == T_DIR ? 0755 : 0644);
  din.nlink = xshort(1);
  din.size = xint(0);
  winode(inum, &din);
  return inum;
}

void balloc(int used)
{
  if (used > FSSIZE) error("image exceeds disk capacity");
  printf("balloc: first %d blocks have been allocated\n", used);
  for (int block = 0; block < nbitmap; ++block)
  {
    uchar bytes[BSIZE]{};
    for (int bit = 0; bit < BPB && block * BPB + bit < used; ++bit)
      bytes[bit / 8] |= 1 << (bit % 8);
    wsect(xint(sb.bmapstart) + block, bytes);
  }
}

uint allocate_block()
{
  if (freeblock >= FSSIZE) error("out of image blocks");
  return freeblock++;
}

constexpr uint min(uint a, uint b) { return a < b ? a : b; }

void
iappend(uint inum, void *xp, int n)
{
  char *p = (char *)xp;
  uint fbn, off, n1;
  struct dinode din;
  char buf[BSIZE];
  uint indirect[NINDIRECT];
  uint x;

  rinode(inum, &din);
  off = xint(din.size);
  // printf("append inum %d at off %d sz %d\n", inum, off, n);
  while (n > 0) {
    fbn = off / BSIZE;
    if (fbn >= MAXFILE) error("host file exceeds maximum xv6 file size");
    if (fbn < NDIRECT) {
      if (xint(din.addrs[fbn]) == 0) {
        din.addrs[fbn] = xint(allocate_block());
      }
      x = xint(din.addrs[fbn]);
    } else {
      if (xint(din.addrs[NDIRECT]) == 0) {
        din.addrs[NDIRECT] = xint(allocate_block());
      }
      rsect(xint(din.addrs[NDIRECT]), (char *)indirect);
      if (indirect[fbn - NDIRECT] == 0) {
        indirect[fbn - NDIRECT] = xint(allocate_block());
        wsect(xint(din.addrs[NDIRECT]), (char *)indirect);
      }
      x = xint(indirect[fbn - NDIRECT]);
    }
    n1 = min(n, (fbn + 1) * BSIZE - off);
    rsect(x, buf);
    bcopy(p, buf + (off - fbn * BSIZE), n1);
    wsect(x, buf);
    n -= n1;
    off += n1;
    p += n1;
  }
  din.size = xint(off);
  winode(inum, &din);
}

void
die(const char *s)
{
  perror(s);
  error("host I/O failed");
}
