#include "application.h"
#include "kernel/support/types.h"
#include "kernel/fs/stat.h"
#include "user/runtime/user.h"
#include "user/fs/backends/xv6/format.h"
#include "kernel/fs/fcntl.h"
#include "kernel/support/param.h"
#include "user/runtime/directory.h"

extern "C"
{

  char *fmtname(char *path);
  static bool detailed = false;

  const char *
  type_name(short type) {
    switch (type)
    {
    case T_DIR:
      return "directory";
    case T_FILE:
      return "file";
    case T_DEVICE:
      return "char-dev";
    case T_BLOCK:
      return "block-dev";
    case T_SYMLINK:
      return "symlink";
    case T_SOCKET:
      return "socket";
    default:
      return "unknown";
    }
  }

  void
  heading() {
    if (detailed)
    {
      printf("%-10s %5s %6s %6s %12s %s\n", "MODE", "LINKS", "UID", "GID", "SIZE(B)",
             "NAME");
    }
    else
    {
      printf("%-9s %8s %12s %s\n", "TYPE", "INODE", "SIZE(B)", "NAME");
    }
  }

  void
  show(char *path, const struct stat &entry) {
    if (!detailed)
    {
      printf("%-9s %8u %12lu %s\n", type_name(entry.type), entry.ino, entry.size,
             fmtname(path));
      return;
    }
    char mode[11];
    mode[0] = entry.type == T_DIR       ? 'd'
              : entry.type == T_DEVICE  ? 'c'
              : entry.type == T_BLOCK   ? 'b'
              : entry.type == T_SYMLINK ? 'l'
                                        : '-';
    const char bits[] = "rwx";
    for (unsigned i = 0; i < 9; ++i)
    {
      mode[i + 1] = entry.mode & (1U << (8 - i)) ? bits[i % 3] : '-';
    }
    if (entry.mode & 04000)
    {
      mode[3] = mode[3] == 'x' ? 's' : 'S';
    }
    if (entry.mode & 02000)
    {
      mode[6] = mode[6] == 'x' ? 's' : 'S';
    }
    if (entry.mode & 01000)
    {
      mode[9] = mode[9] == 'x' ? 't' : 'T';
    }
    mode[10] = 0;
    printf("%-10s %5d %6u %6u %12lu %s\n", mode, entry.nlink, entry.uid, entry.gid,
           entry.size, fmtname(path));
  }

  char *
  fmtname(char *path) {
    char *name = path;
    for (char *p = path; *p; ++p)
    {
      if (*p == '/' && p[1])
      {
        name = p + 1;
      }
    }
    return name;
  }

  void
  ls(char *path) {
    static char buf[MAXPATH];
    char *p;
    int fd;
    const linux_dirent64 *de;
    struct stat st;

    if (lstat(path, &st) == 0 && st.type == T_SYMLINK)
    {
      heading();
      show(path, st);
      return;
    }
    if ((fd = open(path, O_RDONLY)) < 0)
    {
      fprintf(2, "ls: cannot open %s\n", path);
      return;
    }

    if (fstat(fd, &st) < 0)
    {
      fprintf(2, "ls: cannot stat %s\n", path);
      close(fd);
      return;
    }

    heading();
    DirectoryReader reader(fd);
    switch (st.type)
    {
    case T_BLOCK:
    case T_DEVICE:
    case T_FILE:
      show(path, st);
      break;

    case T_DIR:
      if (strlen(path) + 2 > sizeof buf)
      {
        printf("ls: path too long\n");
        break;
      }
      strcpy(buf, path);
      p = buf + strlen(buf);
      *p++ = '/';
      while (reader.next(de) == 1)
      {
        unsigned length = strlen(de->d_name);
        if (unsigned(p - buf) + length + 1 > sizeof(buf))
        {
          fprintf(2, "ls: path too long\n");
          continue;
        }
        memmove(p, de->d_name, length + 1);
        if (lstat(buf, &st) < 0)
        {
          printf("ls: cannot stat %s\n", buf);
          continue;
        }
        show(buf, st);
      }
      break;
    }
    close(fd);
  }

  int
  app_main(int argc, char *argv[]) {
    int i;

    int first = 1;
    if (argc > 1 && strcmp(argv[1], "-l") == 0)
    {
      detailed = true;
      ++first;
    }
    if (argc == first)
    {
      ls(".");
      exit(0);
    }
    for (i = first; i < argc; i++)
    {
      if (argc - first > 1)
      {
        printf("%s%s:\n", i == first ? "" : "\n", argv[i]);
      }
      ls(argv[i]);
    }
    exit(0);
  }
}
