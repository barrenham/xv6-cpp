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
- [`xv::fs::Text::good`](procfs.cpp.md#fn-35b1ca8866c82d1e)
- [`xv::fs::Text::number`](procfs.cpp.md#fn-92a34cf2de670a84)
- [`xv::fs::component_number`](procfs.cpp.md#fn-d517311402dd3534)
- [`xv::fs::file_kind`](procfs.cpp.md#fn-ee0b4055ed93f099)
- [`xv::fs::number_name`](procfs.cpp.md#fn-6a664ab5b71ab094)

<a id="fn-0f0c4ca26414028d"></a>

## `xv::fs::Text::append`

```doxygen
@fn void xv::fs::Text::append(const char *text)
@brief 将文本追加到动态页链并累计失败状态。
@details 计算零终止文本长度后追加，分配或复制失败时记录 failed_。生成方最终检查 good，不把已生成前缀当作完整内容发布。
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
@brief 初始化 procfs 的静态只读节点。
@details 目录 0555、静态文件 0444，属主组均为 root；运行期进程节点在 get 中按目标凭据建立缓存身份。
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
@brief 复制 procfs 节点及权限元数据。
@details 输出合成设备、编号、类型、链接和 uid/gid/mode；路径 stat 不生成文本，打开后的 fstat 由快照后端给出内容大小。
```

<a id="fn-0512bfc7f1743007"></a>

## `xv::fs::ProcFileSystem::lookup_entry`

```doxygen
@fn Inode *xv::fs::ProcFileSystem::lookup_entry(Inode *directory, const char *name, unsigned *)
@brief 解析静态信息、self、PID、status 和 fd 层级。
@details self 在查找时解析为当前 PID 的目录；fd 子项验证编号并复制描述符表确认其仍存在。点目录正确处理 fd 目录的父 PID 目录，挂载根向外跳转由 VFS 处理；每次成功返回新的未锁定节点引用。
```

<a id="fn-603705cd3aba4822"></a>

## `xv::fs::ProcFileSystem::read`

```doxygen
@fn int xv::fs::ProcFileSystem::read(Inode *node, int user, unsigned long destination, unsigned offset, unsigned count)
@brief 为低层内核读取临时生成动态内容。
@details 使用局部 PageBuffer 生成后裁剪读取，离开作用域归还所有页。普通 File 读取其打开时快照，不会每次重新枚举。
```

<a id="fn-0a0d9ec0c1338ec9"></a>

## `xv::fs::Text::Text`

```doxygen
@fn explicit xv::fs::Text::Text(PageBuffer &buffer)
@brief 借用动态页链生成 procfs 文本。
@details 不再接收固定容量指针；内容通过 PageBuffer 按需分配，构造不分配页面。
```

<a id="fn-35b1ca8866c82d1e"></a>

## `xv::fs::Text::good`

```doxygen
@fn bool xv::fs::Text::good() const
@brief 检查整份文本是否完整生成。
@details 任何一次追加失败都会返回 false，不以已生成长度掩盖分配错误。
```

<a id="fn-d517311402dd3534"></a>

## `xv::fs::component_number`

```doxygen
@fn int xv::fs::component_number(const char *name, bool zero = false)
@brief 把目录组件解析为非负或正十进制编号。
@details PID 模式拒绝零，fd 模式允许单个零；两种模式都拒绝前导零、非数字和 int 溢出。输入沿用 DIRSIZ 范围，失败返回 -1。
```

<a id="fn-6a664ab5b71ab094"></a>

## `xv::fs::number_name`

```doxygen
@fn void xv::fs::number_name(unsigned value, char *output)
@brief 把 PID 或 fd 编号格式化为目录名称。
@details 使用固定反向数字数组，输出零终止；调用者提供至少十六字节空间，不申请堆或物理页。
```

<a id="fn-ee0b4055ed93f099"></a>

## `xv::fs::file_kind`

```doxygen
@fn const char *xv::fs::file_kind(const FileInfo &file)
@brief 将已复制的文件种类转换为 procfs 文本标签。
@details 节点按 inode 类型区分 file/directory/device，其他种类为 pipe、terminal、socket 或 unknown。不检查正在运行的后端对象。
```

<a id="fn-90753126637513eb"></a>

## `xv::fs::ProcFileSystem::get`

```doxygen
@fn Inode *xv::fs::ProcFileSystem::get(unsigned kind, unsigned pid, unsigned descriptor)
@brief 取得带目标身份的 procfs 动态节点。
@details 先确认 PID 仍存在，缓存键包括 kind/PID/fd 与有效 uid/gid，避免凭据变化复用旧属性。PID 目录 0555、status 0444、fd 目录 0500、fd 文件 0400；引用发布前完成初始化。
```

<a id="fn-a48454ff4df79560"></a>

## `xv::fs::ProcFileSystem::snapshot`

```doxygen
@fn bool xv::fs::ProcFileSystem::snapshot(Inode *node, PageBuffer &buffer)
@brief 生成进程信息与描述符的打开时快照。
@details status 增加 Uid/Euid/Gid/Egid。fd 枚举调用带实时身份检查的进程服务；跨页 PageBuffer 分配或目标检查失败时返回 false，调用方销毁全部临时内容。
```
