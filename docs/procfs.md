# procfs 与 ps/free

系统在 `/proc` 挂载只读 procfs，把系统与进程信息提供给普通文件接口。`ps`、`free` 只读取 procfs，不新增专用系统调用。

| 路径 | 内容 |
| --- | --- |
| `/proc/meminfo` | MemTotal、MemFree、MemAvailable、MemUsed，单位 kB |
| `/proc/uptime` | 启动后秒数，格式为小数点后两位 |
| `/proc/PID/status` | Name、State、Pid、PPid、Pgid、Sid、VmSize |
| `/proc`、`/proc/PID` | 可用 ls/read 浏览的 dirent 目录 |

```sh
ps
free
cat /proc/meminfo
cat /proc/uptime
ls /proc
```

## 打开时快照

每个独立 open 分配一页并生成内容，read 在固定内容上推进偏移。短读不会重复生成内容，复制失败不推进偏移，fstat 报告本次快照长度。dup/fork 共享同一页和偏移，最后关闭归还页面。

打开的 status 即使对应进程退出并被回收，也能读完原内容；后续新 open 会失败。快照只保存复制的数据，不保存 Process 指针。根目录同样按每次打开生成，ps 允许枚举到的进程在读取 status 前退出并跳过它。

当前快照上限为 4096 字节，根目录扣除固定记录后最多容纳 252 个 PID；容量不足时 open 失败，不发布截断目录。动态节点缓存有 256 个槽，已被引用的槽不会复用。目录 ABI 保留 xv6 的 16 字节 dirent，名称解析按实际 PID 完成。

procfs 禁止写打开、截断、创建、链接和删除，`/proc` 挂载点也受到 VFS 删除保护。相对路径和根目录的 `..` 使用统一 VFS 规则。

## 信息来源

进程信息通过 `snapshot_after` 服务复制。服务取得家族锁保护父指针，再由进程存储逐项加锁，取得名称和状态，组与会话通过组锁查询。回调不睡眠，也不把内部指针交给文件系统。

地址空间由拥有线程继续维护；成功修改后原子发布一个独立的可观测大小。procfs 读取该值，不并发读取页表或拥有线程的普通 size 字段。名称的 exec 更新也使用进程锁，与观测互斥。

MemTotal 是分配器管理的 usable 物理页总量，排除内核、保留区和分配器元数据。两个分配器都按成功分配与归还更新原子空闲页计数；大块分配按页数计量，分裂已分配块不改变空闲总量。MemAvailable 当前等于 MemFree，不估计可回收缓存。VmSize 是进程虚拟地址空间大小，与物理页使用量不同。

uptime 来自系统 TickClock，当前 QEMU virt 每 tick 为 100 ms。

## 验证

`procfstest` 验证内存字段、短读、错误用户地址、只读规则、子进程退出后已有快照存续、共享偏移、相对路径和重复打开。原生 fs_contract 还检查快照更新、复制失败和页面回收；两个 allocator_contract 检查分配、耗尽、碎片归还后的页数统计。

单核/三核集成测试：`ctest --test-dir build/dev -R 'procfs_[13]cpu' --output-on-failure`。函数说明见 [procfs 实现](functions/kernel/fs/procfs/procfs.cpp.md)。
