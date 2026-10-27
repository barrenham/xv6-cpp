#include "kernel/support/types.h"
#include "kernel/fs/stat.h"
#include "user/runtime/user.h"
#include "kernel/fs/format/layout.h"
#include "kernel/fs/fcntl.h"

extern "C" {

char *fmtname(char *path);
static bool detailed = false;
void show(char *path, const struct stat &entry)
{
  if (!detailed) { printf("%s %d %d %d\n", fmtname(path), entry.type, entry.ino, (int)entry.size); return; }
  char mode[11]; mode[0] = entry.type == T_DIR ? 'd' : entry.type == T_DEVICE ? 'c' : entry.type == T_SYMLINK ? 'l' : '-';
  const char bits[] = "rwx";
  for (unsigned i = 0; i < 9; ++i) mode[i + 1] = entry.mode & (1U << (8 - i)) ? bits[i % 3] : '-';
  if (entry.mode & 04000) mode[3] = mode[3] == 'x' ? 's' : 'S';
  if (entry.mode & 02000) mode[6] = mode[6] == 'x' ? 's' : 'S';
  if (entry.mode & 01000) mode[9] = mode[9] == 'x' ? 't' : 'T';
  mode[10] = 0;
  printf("%s %d %d %d %lu %s\n", mode, entry.nlink, entry.uid, entry.gid, entry.size, fmtname(path));
}

char *
fmtname(char *path)
{
  static char buf[DIRSIZ + 1];
  char *p;

  // Find first character after last slash.
  for (p = path + strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if (strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf + strlen(p), ' ', DIRSIZ - strlen(p));
  buf[sizeof(buf) - 1] = '\0';
  return buf;
}

void
ls(char *path)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if (lstat(path, &st) == 0 && st.type == T_SYMLINK) { show(path, st); return; }
  if ((fd = open(path, O_RDONLY)) < 0) {
    fprintf(2, "ls: cannot open %s\n", path);
    return;
  }

  if (fstat(fd, &st) < 0) {
    fprintf(2, "ls: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch (st.type) {
  case T_DEVICE:
  case T_FILE:
    show(path, st);
    break;

  case T_DIR:
    if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
      printf("ls: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf + strlen(buf);
    *p++ = '/';
    while (read(fd, &de, sizeof(de)) == sizeof(de)) {
      if (de.inum == 0)
        continue;
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      if (lstat(buf, &st) < 0) {
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
main(int argc, char *argv[])
{
  int i;

  int first = 1;
  if (argc > 1 && strcmp(argv[1], "-l") == 0) { detailed = true; ++first; }
  if (argc == first) {
    ls(".");
    exit(0);
  }
  for (i = first; i < argc; i++)
    ls(argv[i]);
  exit(0);
}
}
