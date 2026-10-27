#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "kernel/fs/fcntl.h"
#include "kernel/fs/stat.h"
#include "user/runtime/user.h"

static void check(bool good, const char *message)
{
  if (!good)
  {
    fprintf(2, "workflowtest: FAIL: %s\n", message);
    exit(1);
  }
}

static void put(const char *path, const char *text)
{
  int fd = open(path, O_CREATE | O_WRONLY | O_TRUNC);
  check(fd >= 0, "create fixture");
  check(write(fd, text, strlen(text)) == static_cast<int>(strlen(text)),
        "write fixture");
  close(fd);
}

static void contents(const char *path, const char *expected)
{
  char text[64]{};
  int fd = open(path, O_RDONLY);
  check(fd >= 0, "open content");
  int n = read(fd, text, sizeof(text) - 1);
  close(fd);
  check(n >= 0 && !strcmp(text, expected), "contents preserved");
}

static void run(const char *program, const char *source, const char *target)
{
  int pid = fork();
  check(pid >= 0, "fork utility");
  if (!pid)
  {
    char *args[] = {const_cast<char *>(program), const_cast<char *>(source),
                    const_cast<char *>(target), nullptr};
    execv(program, args);
    exit(127);
  }
  int status;
  check(wait(&status) == pid && status == 0, "utility status");
}

extern "C" int main(int argc, char **argv)
{
  if (argc == 2 && !strcmp(argv[1], "negative"))
  {
    exit(-1);
  }
  check(mkdir("/tmp/workflow") == 0, "workspace");
  check(chdir("/tmp/workflow") == 0, "workspace cwd");
  put("first", "original");
  put("second", "old target");
  int old = open("second", O_RDONLY);
  check(old >= 0, "open replaced file");
  check(rename("first", "second") == 0, "atomic replace");
  contents("second", "original");
  char previous[16]{};
  check(read(old, previous, sizeof(previous)) == 10 && !strcmp(previous, "old target"),
        "open replaced inode survives");
  close(old);
  check(link("second", "alias") == 0 && rename("second", "alias") == 0,
        "hardlink rename no-op");
  contents("second", "original");
  check(mkdir("from") == 0 && mkdir("to") == 0 && mkdir("from/child") == 0,
        "directory fixtures");
  check(rename("from", "from/child/cycle") < 0, "reject directory cycle");
  check(chdir("from/child") == 0, "cwd follows rename");
  check(rename("/tmp/workflow/from/child", "/tmp/workflow/to/child") == 0,
        "move directory");
  char path[MAXPATH];
  check(getcwd(path, sizeof(path)) && !strcmp(path, "/tmp/workflow/to/child"),
        "cwd renamed path");
  check(chdir("/tmp/workflow") == 0, "restore cwd");
  check(rename("to/child", "to") < 0, "reject replacing parent with child");
  run("/bin/cp", "second", "copy");
  contents("copy", "original");
  run("/bin/mv", "copy", "moved");
  contents("moved", "original");
  check(mkdir("mnt") == 0, "mount directory");
  put("mnt/hidden", "underlying");
  check(mount("ramfs", "/tmp/workflow/mnt") == 0, "dynamic RAM mount");
  struct stat st;
  check(stat("mnt/hidden", &st) < 0, "covered contents hidden");
  put("mnt/data", "mounted");
  old = open("mnt/data", O_RDONLY);
  check(old >= 0, "mounted open file");
  check(umount("/tmp/workflow/mnt") < 0, "open file prevents unmount");
  close(old);
  check(chdir("mnt") == 0 && umount("/tmp/workflow/mnt") < 0, "cwd prevents unmount");
  check(chdir("..") == 0, "cross mount parent");
  check(rename("mnt", "movedmnt") < 0 && rename("/tmp/workflow", "/tmp/movedwf") < 0,
        "protect mount and ancestor");
  check(mkdir("mnt/nested") == 0 && mount("ramfs", "/tmp/workflow/mnt/nested") == 0,
        "nested mount");
  check(umount("/tmp/workflow/mnt") < 0, "child mount prevents parent unmount");
  check(umount("/tmp/workflow/mnt/nested") == 0, "unmount child");
  check(umount("/tmp/workflow/mnt") == 0, "unmount populated filesystem");
  contents("mnt/hidden", "underlying");
  for (int i = 0; i < 12; ++i)
  {
    check(mount("ramfs", "/tmp/workflow/mnt") == 0, "reuse mount slot");
    put("mnt/file", "transient");
    check(umount("/tmp/workflow/mnt") == 0, "reclaim populated instance");
  }
  check(umount("/dev") < 0 && umount("/tmp") < 0, "protect system mounts");
  int pid = fork();
  check(pid >= 0, "fork permission check");
  if (!pid)
  {
    check(setgid(1000) == 0 && setuid(1000) == 0, "drop privilege");
    check(mount("ramfs", "/tmp/workflow/mnt") < 0 && umount("/dev") < 0,
          "mount privilege");
    exit(0);
  }
  int status;
  check(wait(&status) == pid && status == 0, "permission child");
  // Exercise disk rename as well as RAM rename, including a different parent.
  check(mkdir("/root/wfsource") == 0 && mkdir("/root/wftarget") == 0,
        "disk directories");
  put("/root/wfsource/a", "disk");
  put("/root/wftarget/b", "replace");
  check(rename("/root/wfsource/a", "/root/wftarget/b") == 0, "disk atomic replacement");
  contents("/root/wftarget/b", "disk");
  check(mkdir("/root/wfsource/sub") == 0 &&
            rename("/root/wfsource/sub", "/root/wftarget/sub") == 0,
        "disk directory move");
  check(chdir("/root/wftarget/sub") == 0 && chdir("..") == 0 &&
            getcwd(path, sizeof(path)) && !strcmp(path, "/root/wftarget"),
        "disk parent updated");
  check(chdir("/") == 0, "exit cwd");
  check(rename("/root/wftarget/sub", "/root/wftarget") < 0,
        "disk parent alias rejected");
  printf("workflowtest: all tests succeeded\n");
  exit(0);
}
