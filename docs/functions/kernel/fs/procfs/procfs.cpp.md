# `kernel/fs/procfs/procfs.cpp`

## 函数导航

- [`xv::fs::ProcFileSystem::get`](procfs.cpp.md#fn-90753126637513eb)
- [`xv::fs::ProcFileSystem::initialize`](procfs.cpp.md#fn-74b43972b16a8a28)
- [`xv::fs::ProcFileSystem::initialize_inodes`](procfs.cpp.md#fn-731ba950b1042b0b)
- [`xv::fs::ProcFileSystem::lock`](procfs.cpp.md#fn-9b990852215e1514)
- [`xv::fs::ProcFileSystem::lookup_entry`](procfs.cpp.md#fn-0512bfc7f1743007)
- [`xv::fs::ProcFileSystem::read`](procfs.cpp.md#fn-603705cd3aba4822)
- [`xv::fs::ProcFileSystem::release`](procfs.cpp.md#fn-0072b177352873a8)
- [`xv::fs::ProcFileSystem::retain`](procfs.cpp.md#fn-d20670821de275d2)
- [`xv::fs::ProcFileSystem::snapshot`](procfs.cpp.md#fn-a48454ff4df79560)
- [`xv::fs::ProcFileSystem::stat`](procfs.cpp.md#fn-1eba63efb443f71d)
- [`xv::fs::ProcFileSystem::truncate`](procfs.cpp.md#fn-188d230713358c4d)
- [`xv::fs::ProcFileSystem::unlock`](procfs.cpp.md#fn-441bd17e918e61e8)
- [`xv::fs::ProcFileSystem::unlock_release`](procfs.cpp.md#fn-df886b5ac3282c5b)
- [`xv::fs::ProcFileSystem::update`](procfs.cpp.md#fn-0f8f2177ab9f8594)
- [`xv::fs::Text::Text`](procfs.cpp.md#fn-0a0d9ec0c1338ec9)
- [`xv::fs::Text::append`](procfs.cpp.md#fn-0f0c4ca26414028d)
- [`xv::fs::Text::field`](procfs.cpp.md#fn-4ccdd58043fe99ba)
- [`xv::fs::Text::number`](procfs.cpp.md#fn-92a34cf2de670a84)
- [`xv::fs::Text::size`](procfs.cpp.md#fn-7c67ec963fec4fe8)
- [`xv::fs::pid_name`](procfs.cpp.md#fn-3771c76d893a1ad0)

<a id="fn-0a0d9ec0c1338ec9"></a>

## `xv::fs::Text::Text`

```doxygen
@fn xv::fs::Text::Text(char *buffer, unsigned capacity)
@brief 借用一个有容量上限的 procfs 内容缓冲。
@details 构造清空写入位置和失败标记，不分配空间，不转移缓冲所有权。
```

<a id="fn-0f0c4ca26414028d"></a>

## `xv::fs::Text::append`

```doxygen
@fn void xv::fs::Text::append(const char *text)
@brief 把零终止文本追加到有界缓冲。
@details 逐字节检查容量，容量不足置失败标记；缓冲内容不额外追加终止符，snapshot 根据实际长度返回。
```

<a id="fn-92a34cf2de670a84"></a>

## `xv::fs::Text::number`

```doxygen
@fn void xv::fs::Text::number(unsigned long value)
@brief 追加无符号十进制数字。
@details 用固定反向数字缓冲处理包括零在内的 unsigned long，再通过 append 输出，溢出由共享失败标记记录。
```

<a id="fn-4ccdd58043fe99ba"></a>

## `xv::fs::Text::field`

```doxygen
@fn void xv::fs::Text::field(const char *name, unsigned long value, const char *unit = "\n")
@brief 追加字段名、十进制数值与单位或换行。
@details 用于 meminfo 和 status 格式，调用 append/number 的顺序固定，共享容量限制。
```

<a id="fn-7c67ec963fec4fe8"></a>

## `xv::fs::Text::size`

```doxygen
@fn int xv::fs::Text::size() const
@brief 返回已生成内容长度或失败。
@details 容量不足时返回 -1，防止把截断内容发布为成功快照；正常返回字节数，不包含隐式零终止符。
```

<a id="fn-3771c76d893a1ad0"></a>

## `xv::fs::pid_name`

```doxygen
@fn int xv::fs::pid_name(const char *name)
@brief 把一个目录组件解析为正 PID。
@details 拒绝空名称、前导零、非数字和超过有符号 int 的值；输入沿用 DIRSIZ 范围，失败返回 -1。
```

<a id="fn-731ba950b1042b0b"></a>

## `xv::fs::ProcFileSystem::initialize_inodes`

```doxygen
@fn void xv::fs::ProcFileSystem::initialize_inodes()
@brief 初始化 procfs 节点缓存的归属与锁。
@details 为 256 个静态槽绑定 owner，建立节点睡眠锁和引用表自旋锁；不读取进程信息。
```

<a id="fn-74b43972b16a8a28"></a>

## `xv::fs::ProcFileSystem::initialize`

```doxygen
@fn void xv::fs::ProcFileSystem::initialize(int device)
@brief 建立 procfs 根及 meminfo、uptime 元数据。
@details 保存 stat 使用的文件系统设备号，前三个槽分别代表根目录和两个静态文件。PID 目录及 status 后续按需从缓存取得。
```

<a id="fn-90753126637513eb"></a>

## `xv::fs::ProcFileSystem::get`

```doxygen
@fn Inode *xv::fs::ProcFileSystem::get(unsigned kind, unsigned pid)
@brief 取得指定内容种类和 PID 的缓存节点。
@details 在引用锁内优先复用同一活动节点，否则初始化无人引用的槽。PID 目录与 status 有不同 inode 号；缓存耗尽返回 nullptr，已持引用的槽不会被复用。
```

<a id="fn-d20670821de275d2"></a>

## `xv::fs::ProcFileSystem::retain`

```doxygen
@fn Inode *xv::fs::ProcFileSystem::retain(Inode *node)
@brief 在引用锁内增加 procfs 节点引用。
@details 引用有效期间身份保持稳定；静态节点可从零引用重新取得。
```

<a id="fn-0072b177352873a8"></a>

## `xv::fs::ProcFileSystem::release`

```doxygen
@fn void xv::fs::ProcFileSystem::release(Inode *node)
@brief 在引用锁内归还 procfs 节点引用。
@details 非正引用触发内核错误；动态节点降到零后可被后续 get 复用。内容快照由 File 单独拥有，不在此释放。
```

<a id="fn-9b990852215e1514"></a>

## `xv::fs::ProcFileSystem::lock`

```doxygen
@fn void xv::fs::ProcFileSystem::lock(Inode *node)
@brief 取得有引用的 procfs 节点睡眠锁。
@details 用于元数据、查找和打开快照生成；调用者不得持有自旋锁等待。
```

<a id="fn-441bd17e918e61e8"></a>

## `xv::fs::ProcFileSystem::unlock`

```doxygen
@fn void xv::fs::ProcFileSystem::unlock(Inode *node)
@brief 释放 procfs 节点睡眠锁。
@details 不改变引用，调用者必须持有该锁。
```

<a id="fn-df886b5ac3282c5b"></a>

## `xv::fs::ProcFileSystem::unlock_release`

```doxygen
@fn void xv::fs::ProcFileSystem::unlock_release(Inode *node)
@brief 先释放节点锁再归还一个引用。
@details 顺序遵守公共节点生命周期契约，不使用磁盘事务。
```

<a id="fn-0f8f2177ab9f8594"></a>

## `xv::fs::ProcFileSystem::update`

```doxygen
@fn void xv::fs::ProcFileSystem::update(Inode *)
@brief 拒绝写回只读 procfs 元数据。
@details 调用表示内核路径违反后端约定并触发 panic，正常观测与设备路径不调用此入口。
```

<a id="fn-188d230713358c4d"></a>

## `xv::fs::ProcFileSystem::truncate`

```doxygen
@fn void xv::fs::ProcFileSystem::truncate(Inode *)
@brief 拒绝截断 procfs 节点。
@details 调用触发 panic；sys_open 先按 read_only 契约拒绝截断，正常用户请求不会到达这里。
```

<a id="fn-1eba63efb443f71d"></a>

## `xv::fs::ProcFileSystem::stat`

```doxygen
@fn void xv::fs::ProcFileSystem::stat(Inode *node, Stat *result)
@brief 复制 procfs 合成节点身份和类型。
@details 调用者持节点锁，输出先清零；打开后端另用其快照真实长度填入 size，避免为 stat 重新枚举进程。
```

<a id="fn-0512bfc7f1743007"></a>

## `xv::fs::ProcFileSystem::lookup_entry`

```doxygen
@fn Inode *xv::fs::ProcFileSystem::lookup_entry(Inode *directory, const char *name, unsigned *)
@brief 查找根静态文件、PID 目录或其 status。
@details 调用者持目录锁，点路径返回内部父子引用。数字 PID 和 status 都验证目标仍存在；不保留进程对象指针，已退出并回收的 PID 不再被新查找打开。
```

<a id="fn-a48454ff4df79560"></a>

## `xv::fs::ProcFileSystem::snapshot`

```doxygen
@fn int xv::fs::ProcFileSystem::snapshot(Inode *node, char *buffer, unsigned capacity)
@brief 生成 meminfo、uptime、进程状态或目录内容。
@details 在节点锁内向给定内核缓冲生成完整快照；目录沿用 dirent ABI。状态包含名称、PID/父 PID、组/会话和虚拟大小，内存以 KiB 计；PID 已消失或容量不足返回 -1。当前 File 缓冲为一页，根目录最多容纳 252 个 PID 记录。
```

<a id="fn-603705cd3aba4822"></a>

## `xv::fs::ProcFileSystem::read`

```doxygen
@fn int xv::fs::ProcFileSystem::read(Inode *node, int user, unsigned long destination, unsigned offset, unsigned count)
@brief 为低层内核读取临时生成内容并复制所需范围。
@details 申请一页临时缓冲，生成快照、裁剪 offset/count 并 copy_out，最后归还页面。普通 File 读取使用打开时快照，不会每次调用此入口重新生成。
```
