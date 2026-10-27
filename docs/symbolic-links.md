# 符号链接

```sh
ln -s /usr/share/doc/xv6/README /tmp/readme
readlink /tmp/readme
cat /tmp/readme
ls -l /tmp
```

`symlink(target, path)` 在磁盘或 RAM 文件系统创建 T_SYMLINK 节点，保存目标路径字节；创建时不要求目标存在。`readlink(path, buffer, size)` 返回复制的字节数，不自动追加 NUL。`lstat` 查询链接本身，`stat`、普通 open、chdir 和 exec 跟随链接。`O_NOFOLLOW` 拒绝最终组件为链接，包括与 O_CREATE 一起使用时。

VFS 在命名空间锁内逐组件展开链接：相对目标以链接所在目录为起点，绝对目标从根重新开始，再拼接尚未消费的路径。每次查找最多展开 16 个链接，拼接后必须装入 MAXPATH；循环、过长展开或坏节点返回失败。路径可以跨越磁盘、RAM 和挂载点。创建时允许通过悬空链接建立最终普通文件；不能隐式创建缺失的中间目录。

unlink 和 rename 操作链接目录项本身，不删除目标。硬链接的旧路径不跟随最终符号链接，可以给链接 inode 建立别名。权限仍检查每个实际经过目录的搜索权限及目标文件权限，链接本身的权限位不作为绕过授权的入口。

普通 xv6 路径组件仍保留既有 DIRSIZ=14 截断行为；mkfs manifest 和 rename 等已有严格入口继续拒绝超长组件。符号链接不会改变这些原有边界。

getcwd 扫描父目录时用 lstat 匹配真实目录项，避免把更早出现的符号链接别名当作物理路径。

ls 查询目录项时使用 lstat，以 `l` 显示符号链接类型；详细模式同时显示 setuid/setgid 位的 s/S 标记。当前不提供链接所有权专用 lchown 或目录遍历的全部 POSIX 选项。

storagecheck 覆盖相对链、悬空链接创建、循环、readlink 短缓冲、stat/lstat 区别和 O_NOFOLLOW；完整用户回归继续覆盖旧的 14 字节名称行为。
