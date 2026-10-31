# `kernel/fs/permissions.cpp`

## 函数导航

- [`xv::fs::detail::allowed`](permissions.cpp.md#fn-5fed5617bcd785ea)
- [`xv::fs::detail::may_remove`](permissions.cpp.md#fn-b05fe3cdae315db8)

<a id="fn-5fed5617bcd785ea"></a>

## `xv::fs::detail::allowed`

```doxygen
@fn bool xv::fs::detail::allowed(Inode *node, unsigned requested, bool real)
@brief 按当前身份判断节点的请求权限。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```

<a id="fn-b05fe3cdae315db8"></a>

## `xv::fs::detail::may_remove`

```doxygen
@fn bool xv::fs::detail::may_remove(Inode *directory, Inode *target)
@brief 判断父目录是否允许删除指定目标。
@details 先验证父 W/X，再按 sticky 位限制为 root、目录属主或目标属主；调用者持相关节点锁，后端在修改目录前完成检查。
```
