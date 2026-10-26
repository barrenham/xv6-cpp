# `kernel/fs/inode/inode.cppm`

开发职责：文件描述、inode、路径、块缓存与日志事务。

源码：[打开实际文件](../../../../../kernel/fs/inode/inode.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.fs:inode`。

直接模块依赖：`xv.sync`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::fs::Inode::Inode`](inode.cppm.md#fn-74d62c7bb35d2b2a)
- [`xv::fs::Inode::Inode`](inode.cppm.md#fn-ce1d59b251e7f9b6)
- [`xv::fs::Inode::device`](inode.cppm.md#fn-3ab6e7ac1fc633f4)
- [`xv::fs::Inode::device_major`](inode.cppm.md#fn-5a5bffa518341fb7)
- [`xv::fs::Inode::device_minor`](inode.cppm.md#fn-ccd4307dbfd4e34c)
- [`xv::fs::Inode::kind`](inode.cppm.md#fn-8713cbf297ee2c61)
- [`xv::fs::Inode::linked`](inode.cppm.md#fn-be30d22e631437a0)
- [`xv::fs::Inode::number`](inode.cppm.md#fn-1e309501f8f708b5)
- [`xv::fs::Inode::operator=`](inode.cppm.md#fn-79f72614e3968738)
- [`xv::fs::Inode::owner`](inode.cppm.md#fn-43932e0d5580a438)
- [`xv::fs::Inode::size_bytes`](inode.cppm.md#fn-24de649f4aa36caa)

<a id="fn-74d62c7bb35d2b2a"></a>

## `xv::fs::Inode::Inode`

```doxygen
@fn constexpr xv::fs::Inode::Inode()
@brief 构造缓存 inode 对象，建立本重载要求的成员初值和借用关系。
@details 使用编译器生成的默认成员行为。普通标量是否被初始化取决于成员默认初始化器和调用方的值初始化方式，不能把 default 一概当作清零。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-ce1d59b251e7f9b6"></a>

## `xv::fs::Inode::Inode`

```doxygen
@fn xv::fs::Inode::Inode(const Inode &)
@brief 缓存 inode 对象的禁用操作：在编译期拒绝此种复制或赋值。
@details 该成员显式声明为 deleted，不存在可在运行期调用的实现。它阻止绕开对象的引用/锁/所有权规则创建第二个拥有者。
@return 无运行期返回路径；使用该操作会导致编译失败。
```

<a id="fn-79f72614e3968738"></a>

## `xv::fs::Inode::operator=`

```doxygen
@fn Inode &xv::fs::Inode::operator=(const Inode &)
@brief 缓存 inode 对象的禁用操作：在编译期拒绝此种复制或赋值。
@details 该成员显式声明为 deleted，不存在可在运行期调用的实现。它阻止绕开对象的引用/锁/所有权规则创建第二个拥有者。
@return 无运行期返回路径；使用该操作会导致编译失败。
```

<a id="fn-8713cbf297ee2c61"></a>

## `xv::fs::Inode::kind`

```doxygen
@fn short xv::fs::Inode::kind() const
@brief 缓存 inode 对象：读取或计算 `type`，返回当前查询结果。
@return 各分支实际返回：`type`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-5a5bffa518341fb7"></a>

## `xv::fs::Inode::device_major`

```doxygen
@fn short xv::fs::Inode::device_major() const
@brief 缓存 inode 对象：读取或计算 `major`，返回当前查询结果。
@return 各分支实际返回：`major`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-3ab6e7ac1fc633f4"></a>

## `xv::fs::Inode::device`

```doxygen
@fn unsigned xv::fs::Inode::device() const
@brief 缓存 inode 对象：读取或计算 `dev`，返回当前查询结果。
@return 各分支实际返回：`dev`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-1e309501f8f708b5"></a>

## `xv::fs::Inode::number`

```doxygen
@fn unsigned xv::fs::Inode::number() const
@brief 缓存 inode 对象：读取或计算 `inum`，返回当前查询结果。
@return 各分支实际返回：`inum`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-24de649f4aa36caa"></a>

## `xv::fs::Inode::size_bytes`

```doxygen
@fn unsigned xv::fs::Inode::size_bytes() const
@brief 缓存 inode 对象：读取或计算 `size`，返回当前查询结果。
@return 各分支实际返回：`size`。应结合上面的条件检查区分正常值、空值和错误值。
```
<a id="fn-43932e0d5580a438"></a>

## `xv::fs::Inode::owner`

```doxygen
@fn FileSystem &xv::fs::Inode::owner() const
@brief 取得该节点所属的文件系统实例。
@details owner_ 在 inode 表初始化时绑定，此后节点复用不改变归属。VFS 按该实例转发引用、锁、元数据与读写操作；不根据设备号猜测后端。
```

<a id="fn-ccd4307dbfd4e34c"></a>

## `xv::fs::Inode::device_minor`

```doxygen
@fn short xv::fs::Inode::device_minor() const
@brief 取得设备节点的 minor 号。
@details 调用者持有 inode 锁或已取得不可变 devfs 节点；与 major 一起用于注册表精确分派。
```

<a id="fn-be30d22e631437a0"></a>

## `xv::fs::Inode::linked`

```doxygen
@fn bool xv::fs::Inode::linked() const
@brief 判断节点是否仍有目录链接。
@details 在 inode 锁内读取 nlink；路径遍历拒绝进入已经删除的目录。此值不是内存引用计数。
```
