module xv.process:elf_format;

namespace xv::process
{
// Format of an ELF executable file

inline constexpr unsigned ELF_MAGIC = 0x464C457FU; // "\x7FELF" in little endian

// File header
struct elfhdr
{
  unsigned magic; // must equal ELF_MAGIC
  unsigned char elf[12];
  unsigned short type;
  unsigned short machine;
  unsigned version;
  unsigned long entry;
  unsigned long phoff;
  unsigned long shoff;
  unsigned flags;
  unsigned short ehsize;
  unsigned short phentsize;
  unsigned short phnum;
  unsigned short shentsize;
  unsigned short shnum;
  unsigned short shstrndx;
};

// Program section header
struct proghdr
{
  unsigned type;
  unsigned flags;
  unsigned long off;
  unsigned long vaddr;
  unsigned long paddr;
  unsigned long filesz;
  unsigned long memsz;
  unsigned long align;
};

// Values for Proghdr type
inline constexpr unsigned ELF_PROG_LOAD = 1;

// Flag bits for Proghdr flags
inline constexpr unsigned ELF_PROG_FLAG_EXEC = 1;
inline constexpr unsigned ELF_PROG_FLAG_WRITE = 2;
inline constexpr unsigned ELF_PROG_FLAG_READ = 4;
static_assert(sizeof(elfhdr) == 64);
static_assert(sizeof(proghdr) == 56);
} // namespace xv::process
