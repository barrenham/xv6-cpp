#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "kernel/fs/format/layout.h"
#include "kernel/fs/stat.h"
#include "kernel/fs/fcntl.h"
#include "kernel/fs/poll.h"
#include "kernel/fs/io_status.h"
#include "kernel/io/block.h"
#include "kernel/io/dirent.h"
#include "kernel/tty/termios.h"
#include "user/runtime/proc.h"
#include "user/runtime/user.h"
namespace
{
void check(bool good, const char *message)
{
  if (!good) { fprintf(2, "devfstest: FAIL: %s\n", message); exit(1); }
}
bool entropy_present=true;
void entropy_exact(int fd,unsigned char *bytes,unsigned count)
{
  unsigned done=0;while(done<count)
  {int part=read(fd,bytes+done,count-done);check(part>0,"complete entropy short reads");done+=part;}
}
void random_devices()
{
  int random=open("/dev/random",O_RDONLY|O_NONBLOCK),urandom=open("/dev/urandom",O_RDONLY|O_NONBLOCK);
  check(random>=0 && urandom>=0,"open registered entropy devices");
  unsigned char first[64],second[64],bytes[701];pollfd ready{random,POLLIN,0};
  if(!entropy_present)
  {
    check(read(random,first,sizeof(first))==-1 && poll(&ready,1,0)==1 && (ready.revents&POLLERR),"absent entropy device reports unavailable");
    close(random);close(urandom);return;
  }
  check(poll(&ready,1,1000)==1 && (ready.revents&POLLIN),"entropy completion readiness");
  check(fcntl(random,F_SETFL,0)==0 && fcntl(urandom,F_SETFL,0)==0,"blocking entropy descriptors");
  entropy_exact(random,first,sizeof(first));entropy_exact(urandom,second,sizeof(second));
  check(memcmp(first,second,sizeof(first))!=0,"separate entropy reads do not replay a buffer");
  for(unsigned round=0;round<16;++round)
  {
    unsigned done=0;while(done<sizeof(bytes)){int n=read(random,bytes+done,sizeof(bytes)-done);check(n>0,"entropy refill and short read progress");done+=n;}
  }
  check(read(urandom,reinterpret_cast<void *>(~0UL),1)==-1,"entropy bad user address");
  check(write(random,bytes,1)==-1 && ioctl(random,0xdead,0)==-1,"read-only entropy interface");
  close(random);close(urandom);
}
void devices()
{
  random_devices();
  int tty=open("/dev/tty",O_RDWR);check(tty>=0 && tcgetpgrp(tty)==tcgetpgrp(0),"console controlling terminal alias");close(tty);
  int null = open("/dev/null", O_RDWR | O_NONBLOCK);
  int zero = open("/dev/zero", O_RDWR);
  int full = open("/dev/full", O_RDWR);
  check(null >= 0 && zero >= 0 && full>=0, "open builtins");
  char data[301]; memset(data, 7, sizeof(data));
  check(write(null, data, sizeof(data)) == sizeof(data) && read(null, data, 1) == 0,
        "null discard and EOF");
  check(read(zero, data, sizeof(data)) == sizeof(data), "zero read");
  for (char byte : data) check(byte == 0, "zero contents");
  check(write(zero, data, sizeof(data)) == sizeof(data), "zero discard");
  memset(data,1,sizeof(data));check(read(full,data,sizeof(data))==sizeof(data),"full reads zeros");
  for(char byte:data)check(!byte,"full read content");
  check(write(full,data,1)==-1 && write(full,data,0)==0,"full writes fail without space");close(full);
  check(read(zero, reinterpret_cast<void *>(~0UL), 1) == -1, "zero invalid destination");
  check(read(zero, data, 0) == 0 && read(null, data, 0) == 0, "empty read");
  pollfd ready[] = {{null, POLLIN | POLLOUT, 0}, {zero, POLLIN | POLLOUT, 0}};
  check(poll(ready, 2, 0) == 2 && ready[0].revents == (POLLIN | POLLOUT) &&
        ready[1].revents == (POLLIN | POLLOUT), "readiness");
  check(ioctl(zero, 0xdead, reinterpret_cast<unsigned long>(data)) == -1, "unsupported ioctl");
  check((fcntl(null, F_GETFL, 0) & O_NONBLOCK) != 0, "nonblock flag");
  int copy = dup(null); check(copy >= 0, "dup"); close(null);
  check(write(copy, data, 1) == 1, "duplicate lifetime");
  int child = fork(); check(child >= 0, "fork");
  if (child == 0)
  {
    check(read(zero, data, sizeof(data)) == sizeof(data) && write(copy, data, 1) == 1,
          "inherited file descriptors");
    close(copy); close(zero); exit(0);
  }
  int status = -1; check(wait(&status) == child && status == 0, "child device use");
  close(copy); close(zero);
}
void block_devices()
{
  int fd=open("/dev/vda",O_RDONLY);check(fd>=0,"open root block device");
  struct stat metadata{};unsigned long bytes=0;int sector=0,readonly=0;
  check(fstat(fd,&metadata)==0 && metadata.type==T_BLOCK,"block node metadata");
  check(ioctl(fd,BLKGETSIZE64,reinterpret_cast<unsigned long>(&bytes))==0 && bytes==metadata.size && bytes>2048,"block capacity matches fstat");
  check(ioctl(fd,BLKSSZGET,reinterpret_cast<unsigned long>(&sector))==0 && sector==512 &&
    ioctl(fd,BLKROGET,reinterpret_cast<unsigned long>(&readonly))==0 && readonly==1,"block geometry and protection");
  check(open("/dev/vda",O_RDWR)<0,"mounted disk rejects raw writable open");
  check(lseek(fd,1024,SEEK_SET)==1024,"raw superblock seek");
  unsigned char super[64];check(read(fd,super,sizeof(super))==sizeof(super),"raw superblock read");
  unsigned magic=super[0]|(unsigned(super[1])<<8)|(unsigned(super[2])<<16)|(unsigned(super[3])<<24);
  check(magic==FSMAGIC || (super[56]==0x53 && super[57]==0xef),"raw superblock matches ext3 or xv6");
  int alias=dup(fd);check(alias>=0 && lseek(alias,-2,SEEK_END)==long(bytes-2),"raw dup shares seek");
  check(read(fd,super,sizeof(super))==2 && read(fd,super,1)==0,"raw bounded EOF");
  check(ioctl(fd,BLKGETSIZE64,~0UL)==-1 && lseek(fd,1,SEEK_END)==-1,"raw ioctl fault and seek bounds");
  close(alias);close(fd);
  char information[512];check(proc::read("/proc/partitions",information,sizeof(information))>0,"partitions snapshot");
  bool found=false;for(char *at=information;*at;++at)
  {unsigned n=0;while(n<5 && at[n] && at[n]==" vda\n"[n])++n;if(n==5)found=true;}
  check(found,"registered disk appears in partitions");
}
void directories(bool wide)
{
  const char *paths[]={"/","/dev","/proc","/tmp"};
  for(const char *path:paths)
  {
    int fd=open(path,O_RDONLY);check(fd>=0,"native directory open");alignas(8) unsigned char bytes[1024];
    check(getdents64(fd,bytes,1)==-1 && getdents64(fd,reinterpret_cast<void *>(~0UL),sizeof(bytes))==-1,"native directory buffer and copy faults");
    int count=getdents64(fd,bytes,24);auto *entry=reinterpret_cast<linux_dirent64 *>(bytes);
    check(count==24 && !strcmp(entry->d_name,"."),"failed copy keeps directory at first record");
    int alias=dup(fd);check(alias>=0 && getdents64(alias,bytes,24)==24 && !strcmp(entry->d_name,".."),"native directory dup offset");
    unsigned long previous=entry->d_off;unsigned long_names=0;
    while((count=getdents64(fd,bytes,sizeof(bytes)))>0)
    {
      unsigned offset=0;
      while(offset<unsigned(count))
      {
        entry=reinterpret_cast<linux_dirent64 *>(bytes+offset);
        check(entry->d_reclen>=24 && !(entry->d_reclen%8) && entry->d_reclen<=count-offset && entry->d_off>previous,"native record bounds and advancing cookies");
        unsigned length=0;while(DIRENT64_NAME_OFFSET+length<entry->d_reclen && entry->d_name[length])++length;
        check(length<=255 && DIRENT64_NAME_OFFSET+length<entry->d_reclen,"native terminated names");
        for(unsigned i=DIRENT64_NAME_OFFSET+length+1;i<entry->d_reclen;++i)check(bytes[offset+i]==0,"native directory padding");
        if(length==255 && !memcmp(entry->d_name,"enum-",5)){check(entry->d_type==4,"ext3 directory-entry type");++long_names;}
        previous=entry->d_off;offset+=entry->d_reclen;
      }
      check(offset==unsigned(count),"native directory returns whole records");
    }
    check(!count,"native directory EOF");if(wide && !strcmp(path,"/"))check(long_names==2,"full distinct 255-byte ext3 directory names");
    check(read(fd,bytes,16)==-1,"native directory rejects legacy reads");close(alias);close(fd);
    fd=open(path,O_RDONLY);check(fd>=0 && read(fd,bytes,16)==16 && getdents64(fd,bytes,sizeof(bytes))==-1,"legacy directory API remains separate");close(fd);
  }
  int fd=open("/dev/null",O_RDONLY);char byte;check(getdents64(fd,&byte,1)==-1,"non-directory descriptors rejected");close(fd);
}
unsigned content_checksum(const char *path)
{
  int fd=open(path,O_RDONLY);check(fd>=0,"checksum file open");unsigned hash=2166136261U;unsigned char bytes[256];int count;
  while((count=read(fd,bytes,sizeof(bytes)))>0)for(int i=0;i<count;++i)hash=(hash^bytes[i])*16777619U;
  check(count==0,"checksum file read");close(fd);return hash;
}
void wide_paths()
{
  static char first[MAXPATH],second[MAXPATH],file[MAXPATH],renamed[MAXPATH],alias[MAXPATH],current[MAXPATH],limit[MAXPATH+1];
  char name[256];memset(name,'x',255);memmove(name,"enum-",5);name[254]='a';name[255]=0;
  first[0]='/';strcpy(first+1,name);name[254]='b';second[0]='/';strcpy(second+1,name);
  struct stat a{},b{};check(stat(first,&a)==0 && stat(second,&b)==0 && a.ino!=b.ino,"wide path names remain distinct");
  strcpy(limit,first);strcpy(limit+strlen(limit),"z");
  check(stat(limit,&b)<0 && open(limit,O_CREATE|O_RDWR)<0 && mkdir(limit)<0 && stat(first,&b)==0 && b.ino==a.ino,"256-byte name cannot alias existing 255-byte name");
  strcpy(file,first);strcpy(file+strlen(file),"/");unsigned prefix=strlen(file);memset(name,'f',255);name[254]='a';name[255]=0;strcpy(file+prefix,name);
  strcpy(renamed,file);renamed[strlen(renamed)-1]='b';strcpy(alias,second);strcpy(alias+strlen(alias),"/hard");
  int fd=open(file,O_CREATE|O_RDWR);check(fd>=0 && write(fd,"wide",4)==4,"create and write nested wide path");close(fd);
  check(link(file,alias)==0 && rename(file,renamed)==0 && stat(file,&b)<0 && stat(renamed,&b)==0 && b.nlink==2,"wide link and rename");
  check(symlink(renamed,"/wide-link")==0 && readlink("/wide-link",current,sizeof(current))==int(strlen(renamed)),"wide symlink target");
  current[strlen(renamed)]=0;check(!strcmp(current,renamed),"read full symlink target");
  fd=open("/wide-link",O_RDONLY);char data[4];check(fd>=0 && read(fd,data,sizeof(data))==4 && !memcmp(data,"wide",4),"resolve wide symlink");close(fd);
  check(chdir(first)==0 && getcwd(current,sizeof(current)) && !strcmp(current,first) && chdir("/")==0,"wide physical working directory");
  check(unlink("/wide-link")==0 && unlink(alias)==0 && unlink(renamed)==0,"wide file cleanup");
  check(link("/bin/pwd",file)==0,"wide executable hard link");
  int child=fork();check(child>=0,"wide executable fork");
  if(!child){char *arguments[]={const_cast<char *>("pwd"),nullptr};execv(file,arguments);exit(1);}
  int status;check(wait(&status)==child && status==0 && unlink(file)==0,"execute ELF through a 512-byte path");
  strcpy(file,"/wide-tree");check(mkdir(file)==0,"deep wide tree root");unsigned used=strlen(file);
  memset(name,'n',180);name[180]=0;
  for(unsigned depth=0;depth<16;++depth)
  {name[0]='A'+depth;file[used++]='/';strcpy(file+used,name);used+=180;check(mkdir(file)==0,"deep wide directory");}
  check(strlen(file)>2800 && chdir(file)==0 && getcwd(current,sizeof(current)) && !strcmp(current,file) && chdir("/")==0,"multi-kilobyte physical cwd");
  for(unsigned depth=0;depth<16;++depth){check(unlink(file)==0,"deep directory cleanup");used-=181;file[used]=0;}check(unlink(file)==0,"deep tree root cleanup");
  const char *target="/bin/echo";unsigned length=strlen(target);
  memset(limit,'/',MAXPATH-1-length);strcpy(limit+MAXPATH-1-length,target);
  fd=open(limit,O_RDONLY);check(fd>=0,"4095-byte syscall path boundary");close(fd);
  memset(limit,'/',MAXPATH-length);strcpy(limit+MAXPATH-length,target);check(open(limit,O_RDONLY)<0,"4096-byte syscall path rejected");
}
void failed_truncate()
{
  int before=open("/dev/null",O_RDONLY);check(before>=0,"truncate rollback descriptor baseline");close(before);
  struct stat old{},current{};check(stat("/bin/echo",&old)==0 && old.size>0,"truncate failure fixture");
  unsigned checksum=content_checksum("/bin/echo");
  for(unsigned i=0;i<400;++i)check(open("/bin/echo",O_WRONLY|O_TRUNC)==-1,"failed truncate returns error without publishing descriptor");
  int after=open("/dev/null",O_RDONLY);check(after==before,"failed truncate releases descriptor and file slots");close(after);
  check(stat("/bin/echo",&current)==0 && current.ino==old.ino && current.size==old.size && current.mode==old.mode,"failed truncate preserves inode and size");
  check(content_checksum("/bin/echo")==checksum,"failed truncate preserves complete file contents");
  int file=open("/tmp/truncate-ok",O_CREATE|O_RDWR);check(file>=0 && write(file,"abc",3)==3,"successful truncate fixture");close(file);
  file=open("/tmp/truncate-ok",O_WRONLY|O_TRUNC);check(file>=0 && fstat(file,&current)==0 && current.size==0,"successful truncate remains usable");close(file);check(unlink("/tmp/truncate-ok")==0,"truncate fixture cleanup");
}
void paths()
{
  struct stat disk{}, dev{}, node{};
  check(stat("/", &disk) == 0 && stat("/dev", &dev) == 0 && disk.dev != dev.dev &&
        dev.type == T_DIR, "mounted filesystem identity");
  check(stat("/dev/console", &node) == 0 && node.type == T_DEVICE && node.dev == dev.dev,
        "console metadata");
  int dir = open("/dev", O_RDONLY); check(dir >= 0, "open directory");
  dirent entry{}; unsigned found = 0; int count;
  while ((count = read(dir, &entry, sizeof(entry))) == sizeof(entry))
  {
    if (!strcmp(entry.name, "console")) found |= 1;
    if (!strcmp(entry.name, "null")) found |= 2;
    if (!strcmp(entry.name, "zero")) found |= 4;
  }
  check(count == 0 && found == 7, "directory listing"); close(dir);
  check(chdir("/dev") == 0 && stat("./zero", &node) == 0 && node.type == T_DEVICE,
        "devfs working directory");
  check(chdir("..") == 0 && stat(".", &node) == 0 && node.dev == disk.dev && node.ino == disk.ino,
        "escape mounted root");
  check(stat("/dev/../dev//./null", &node) == 0 && node.type == T_DEVICE, "cross-mount normalization");
  check(unlink("/dev") < 0 && unlink("/dev/../dev") < 0 && unlink("/dev/null") < 0,
        "mount and nodes cannot be removed");
  check(mkdir("/dev/new") < 0 && mknod("/dev/new", 1, 0) < 0 &&
        open("/dev/new", O_CREATE | O_RDWR) < 0, "immutable namespace");
  check(link("/dev/null", "/devlink") < 0, "cross-filesystem hard link");
  int file = open("/dev-disk", O_CREATE | O_RDWR); check(file >= 0, "disk creation"); close(file);
  check(link("/dev-disk", "/dev/disk") < 0 && unlink("/dev-disk") == 0, "disk to devfs link rejected");
  file = open("/dev/zero", O_CREATE | O_RDWR | O_TRUNC);
  check(file >= 0, "create opens existing device"); close(file);
  file = open("/dev/console", O_RDWR | O_NONBLOCK); check(file >= 0, "open console"); close(file);
  check(open("/dev", O_RDWR) < 0, "directory write rejected");
}
}
extern "C" int main(int argc,char **argv)
{
  bool truncate_error=false,wide=false;
  for(int i=1;i<argc;++i)
  {if(!strcmp(argv[i],"noentropy"))entropy_present=false;else if(!strcmp(argv[i],"truncate-error"))truncate_error=true;else if(!strcmp(argv[i],"wide-dir"))wide=true;}
  if(truncate_error)failed_truncate();
  directories(wide);
  if(wide)wide_paths();
  paths(); devices(); block_devices();
  for (int i = 0; i < 400; ++i) { int fd = open("/dev/null", O_RDWR); check(fd >= 0, "repeated open"); close(fd); }
  int children[4];
  for (int &child : children)
  {
    child = fork(); check(child >= 0, "concurrent fork");
    if (child == 0) { devices(); exit(0); }
  }
  for (int child : children) { (void)child; int status; check(wait(&status) > 0 && status == 0, "concurrent child"); }
  printf("devfstest: all tests succeeded\n"); exit(0);
}
