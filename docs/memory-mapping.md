# 内存映射

应用使用 `<sys/mman.h>` 的 `mmap/munmap/mprotect/msync`，错误采用
`MAP_FAILED` 或 `-1/errno`。支持匿名内存、POSIX 共享内存对象和普通文件。

| 来源 | MAP_SHARED | MAP_PRIVATE | 执行权限 |
| --- | --- | --- | --- |
| MAP_ANONYMOUS | fork 后共享零填充页 | 私有页，fork 后隔离 | 支持 PROT_EXEC |
| shm_open 对象 | 按名称独立打开后共享页 | 创建时复制，fork 后隔离 | 不提供执行映射 |
| 普通文件 | 相同 inode/页号共用缓存页 | 首次访问加载，首次写入复制，修改不写回文件 | 支持 PROT_EXEC |

普通文件后端须通过 V2 传输声明 `CacheMetadata`，承诺内容和属性变更通过
VFS 的标准读写/截断入口执行。当前 ramfs、ext3、xv6 已声明，动态 proc/devfs 不声明；旧协议
和未声明能力的独立 FS ELF 仍可读写，但普通文件 mmap 返回 ENODEV。

## 应用示例

```cpp
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

int fd = open("/tmp/mapped", O_CREAT | O_RDWR, 0600);
ftruncate(fd, 4096); // 先建立需要映射的文件长度。
char *data = (char *)mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
close(fd);         // 映射持有独立 inode 引用。
data[0] = 'x';
msync(data, 4096, MS_SYNC);
munmap(data, 4096);
```

每次映射最多 1 MiB，每地址空间最多 32 个映射片段，部分解除映射会占用额外
片段。文件偏移须按页对齐；地址参数作为提示，不支持 MAP_FIXED。必须明确
选择 MAP_SHARED 或 MAP_PRIVATE。普通文件按需加载，私有文件页和 fork 后的私有映射使用 COW。
尚未形成私有副本的文件页可能看到文件缓存的后续修改，不提供 mmap 时刻的整文件快照。

## 权限与生命周期

PROT_NONE 保留物理帧，VMA 和无效叶项共同拒绝访问；恢复权限复用原有内容。
mprotect 覆盖 mmap 区域，先检查整个范围及权限上限，在临时地址树中完成
拆分后提交。跨空洞失败不会修改前面页面的权限。RISC-V 的写权限同时允许读。

只读描述符不能创建可写共享映射，也不能通过 mprotect 获取写权限；私有映射
可通过只读描述符建立并获得私有写权限。可写文件共享映射启用和后续写回会
清除 set-id 位。PROT_EXEC 映射可执行生成代码，也可作为用户信号处理函数。

相同 inode 的共享映射互相可见，普通 read 也能读取尚未写回的映射修改；
普通 write 更新相应缓存页。映射不改变描述符偏移。部分 munmap 保留两侧
正确的文件页偏移。fork 保留共享引用、复制私有页，且各进程的保护独立。

关闭描述符、删除路径或重命名不会撤销已有映射。退出在可睡眠上下文先释放
文件映射，再关闭描述符并进入 zombie；exec 替换旧地址空间时也释放映射。
失败的 fork 清理保留父进程的引用，不在进程锁下执行最后一次写回。

## 写回与文件边界

msync(..., MS_SYNC) 同步提交共享映射；fsync 和挂载级 sync 也会先发布映射
数据，再请求后端提交。MS_INVALIDATE 可与 MS_SYNC 同用；当前不丢弃仍在
使用的缓存页。MS_ASYNC 返回 EOPNOTSUPP。私有映射的 msync 不把私有修改写回。

最后一个映射引用释放时会尽力写回。显式同步发生 EIO 时保留现有数据供重试；
失联后端无法提供持久性保证。缓存按 inode/文件页管理，普通 read/write 与 mmap 复用同一缓存。解除映射后的闲置页
每 inode 至多保留 8 页，最后 inode 引用释放时回收缓存；写入采用写穿并保持映射一致性。

普通文件末尾不足一页的尾部最初为零，向该尾部存储不会增长文件或写回超出
文件长度的字节。整页超过 EOF 的用户访问产生可捕获的 SIGBUS。ftruncate、
O_TRUNC 和扩展会更新缓存内容及可访问长度；私有映射被截掉的尾部也会清零。
内核拷贝用户缓冲区前重新检查边界，拒绝借助 syscall 访问已截掉的页面。

截断使用跨 hart 的 epoch 同步：发布新 EOF 后，等待仍以旧 epoch 在用户态
运行的 hart 进入内核。返回用户态前重新检查映射，trampoline 刷新本 hart
的 TLB/指令缓存。等待使用通知与短定时唤醒；持续执行用户代码的进程由定时
抢占推进，不要求应用自己调用 syscall。该机制目前用于普通文件；既有 POSIX
共享内存对象缩短后的访问尚未接入同样的 EOF/SIGBUS 机制。

按需分页、COW、内容缓存与服务管理的完整说明见[本轮扩展](four-extensions.md)。

## 验证

- 原生 `fs_contract`：真实用户 RAM 后端、重叠物理页、描述符一致性、截断/扩展、
  删除后引用、写回 EIO 重试、set-id 和多阶段 OOM 回收。
- `vm_buddy_contract/vm_freelist_contract`：映射引用、分段、fork、保护、EOF 拷贝
  检查，以及页表和后备帧引用计数回到基线。
- `memory_mapping_1cpu/5cpu`：匿名/共享/私有映射、保护恢复、执行权限与信号处理，
  ramfs/ext3/xv6 普通文件的独立 exec、读写一致性、msync/fsync、部分解除映射、
  关闭/删除、退出与 exec 写回、可捕获 SIGBUS、私有尾部清零，以及四个不调用
  syscall 的并发读取进程在截断后的访问撤销。

`build/file-mmap-complete-functional.log` 7/7 通过单核/五核 ramfs/ext3、两种页
分配器、FS/进程契约和 C 头文件。增加 xv6、可执行映射信号处理后的最终整体验证
在 `build/extension-six-final-validation.log` 为 28/28 通过；三后端映射的单核、
五核测试分别用时 28.27 秒和 22.85 秒，临时 ext3 根盘 e2fsck 通过。
