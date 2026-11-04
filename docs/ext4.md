# 独立 ext4 ELF（只读阶段）

`ext4.elf` 是独立用户态文件系统驱动。源码在 `user/fs/backends/ext4/`，入口在 `user/fs/plugins/ext4/`。
所有 ext4 解析都在 ELF 内执行，内核没有 ext4 格式实现或专用类型分支。
它默认不进入引导包或系统盘；可以在内核运行后单独编译、传入和加载，见 [运行时加载](fs-hot-loading.md)。

## 已实现

- extents 映射：inode 内记录、最多五层树深度、外部索引/叶节点、稀疏区和未初始化 extent 的零填充。
- 原有直接/多级间接块映射，以及短符号链接与块存储的长符号链接。
- 1/2/4 KiB 块、128..块大小的合法 inode、32/64 位块组描述符、flex_bg。
- metadata_csum 的超级块、块组、inode bitmap、inode、外部 extent、目录叶块和 htree 元数据校验；
  支持 checksum_seed；旧 gdt_csum 使用 CRC16。
- 普通文件、目录、符号链接、权限与所有者、getdents64 和兼容的旧目录 read 格式。
- 索引目录使用兼容的线性扫描，跳过内部索引记录；没有实现哈希加速。
- `ext4.native` 版本 1 的几何查询，结构在公共 `xvfs/ext4.h`。

磁盘布局和校验依据 [Linux ext4 格式文档](https://www.kernel.org/doc/html/latest/filesystems/ext4/index.html)、
[extent 格式](https://www.kernel.org/doc/html/latest/filesystems/ext4/ifork.html) 与
[元数据校验规范](https://www.kernel.org/doc/html/latest/filesystems/ext4/checksums.html)，并用 e2fsprogs 实际生成的镜像交叉验证。

## 当前拒绝与限制

驱动不调用写盘、分配、修改属性或日志重放。VFS 拒绝写入、截断、创建、删除、改名和修改权限，
只接受 clean 状态、无需恢复且没有待恢复孤儿的镜像。不把“只读挂载”解释为可以偷偷重放日志。

不支持的 incompat/rocompat 特征会明确拒绝，包括 inline_data、加密、casefold、meta_bg、MMP、bigalloc、
大目录扩展、verity 和待恢复 orphan_file。带外部或 inode 内扩展属性的对象也拒绝，避免在尚未实现 ACL 时错误授予访问。
仅解析 Linux inode 布局。标准 VFS 的 32 位编号/长度与路径长度限制仍存在，驱动拒绝无法表示的对象，
而非把超过 4 GiB 的文件伪装成完整可读。旧 read(dir) 另受 16 位 inode 编号限制，大编号目录应使用 getdents64。

校验和保护元数据，不是数据内容认证。应在卸载状态下修改源镜像；文件源不能在挂载期间由其他进程改写。
很大目录的线性查找可能触及现有 RPC 时间限制，后续应增加 htree 哈希查找。写入和 JBD2 恢复尚未实现。

## 独立构建和测试

```sh
cmake --build build/dev --target user_ext4_plugin -j 1
ctest --test-dir build/dev -R '^(ext4_volume_contract|fs_ext4_|fs_hotplug_)' -j 1 --output-on-failure
```

编译只产生驱动，既不重新链接内核，也不自动安装它。源码 kit 可在仓库外、运行中的 QEMU 旁独立构建。
`XV_INSTALL_EXT4_PLUGIN=ON` 只用于明确需要把它预装到新生成镜像的场景，不是运行时加载的前提。
根文件系统若要使用它，需在管理员自行打包的引导资产中提供相应入口及可只读启动的用户程序；默认启动路径不依赖 ext4。

原生测试使用五种独立布局：三种块大小、64 位/传统描述符、两种元数据校验与 checksum_seed。
包含实际碎片化 extent 树、1600 项索引目录、长短链接、底层填入非零字节的 unwritten extent、
内存分配失败回滚，以及校验和损坏和合法校验和下的非法树深度/指针/几何等拒绝检查。
QEMU 测试覆盖两种块大小和单核/五核，执行 ext4 中的 ELF，验证权限、只读拒绝、重复挂载及数据盘逐字节不变。
