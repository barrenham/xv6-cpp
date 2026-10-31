# 创建与更新 rootfs

工具使用 POSIX 文件锁，需要在 Linux/WSL 等支持该接口的环境运行。

构建产物 `build/dev/fs.img` 是无账户状态的安装种子；`data/rootfs.img` 是运行中的持久根盘。
更新内核只需重启 QEMU，更新已安装的用户程序使用 `rootfs-update`。两者都不需要重新创建用户数据。

## 更新现有程序

先在客体运行 `sync`，再退出使用这块根盘的 QEMU（Ctrl-a 后按 x），在项目根目录运行：

```sh
conda activate OS-LAB
cmake --preset dev
cmake --build build/dev --target rootfs-update
cmake --build build/dev --target qemu
```

目标先构建最新安装种子，再离线更新 `XV_ROOTFS_IMAGE` 指向的已有磁盘。
只替换构建清单中的 `/bin`、`/sbin`、`/usr/tests` 和 `/usr/share/doc` 文件，
安装 UID/GID 为 root，权限按清单设置，包括 passwd 的 04755。
这些文件的系统父目录也按清单恢复 root 属主与目录权限；父目录不能是符号链接。

`/etc` 的账户、密码和组记录，以及 `/root`、`/home` 等清单外文件保持不变。
清单外的自定义程序也保留；已经从清单删除的旧程序不会自动删除。
清单内文件的本地修改会被新构建替换，旧版本保存在完整备份中。
系统文件若与用户文件硬链接，更新只替换系统名称，用户别名保留旧内容；
系统文件若是符号链接，替换该链接本身，不写入链接目标。

工具锁住原镜像并检查文件身份，拒绝正在被 QEMU 使用的磁盘。
所有调整先在内存副本中完成并校验磁盘分配，容量不足或路径冲突不会改动原盘。
发布前先生成同目录完整备份，例如：

```text
rootfs.img.before-update-20261027T000000Z-a1b2c3d4.img
```

备份保存更新前的原始字节，包含账户数据，权限为 0600；随后 fsync 并原子替换根盘。
新文件在发布期间也保持排他锁。提交日志会在私有副本中重放，备份仍保留原始状态。
若没有程序或目录需要更新，则不重写镜像、不生成备份。准备阶段失败不发布副本，
发布采用整文件原子替换，不会把半套程序留在原盘中。

只检查将更新哪些文件，可直接运行：

```sh
python tools/update_rootfs.py --seed build/dev/fs.img \
  --image data/rootfs.img --manifest build/dev/fs-manifest.tsv --dry-run
```

dry-run 也检查容量与锁，但不写磁盘或备份。工具没有自动清理备份；确认不再需要后自行管理。
根盘布局、inode 数和容量保持原样，这不是扩容、在线更新或完整 fsck 工具。

## 把 user 中的程序加入安装清单

已有程序直接修改相应的 `user/apps/` 或 `user/tests/` 源文件即可。新增应用以 `hello` 为例：

1. 创建 `user/apps/hello/hello.cpp`，使用项目的用户运行库和系统调用接口编写入口。
2. 在 `user/CMakeLists.txt` 的 `set(programs ...)` 中加入 `apps/hello/hello.cpp`。
3. 按上一节运行配置和 `rootfs-update`，然后重新启动 QEMU。
4. 在客体 shell 运行 `hello`；普通应用的安装位置是 `/bin/hello`。

`xv_add_program` 根据清单编译并链接 RISC-V 用户程序，构建产物为 `build/dev/user/_hello`，同时生成 `build/dev/fs-manifest.tsv` 中的安装映射。更新工具根据该映射安装编译产物，源文件和运行库本身不会作为应用复制进去。程序名取源文件的文件名，不含扩展名，因此清单中应避免重名。

当前规则将普通应用安装到 `/bin`，将 init、用户/组管理、netctl、iptables 等指定管理命令安装到 `/sbin`，将测试程序安装到 `/usr/tests`。例如 `curl`、`ping` 和 `clear` 都位于 `/bin`。需要新增管理命令时，还应在同一文件的目标路径判断中登记名称。

如果使用另一块已有根盘，先显式选择它，再更新：

```sh
cmake --preset dev -DXV_ROOTFS_IMAGE="$PWD/data/my-rootfs.img"
cmake --build build/dev --target rootfs-update
cmake --build build/dev --target qemu
```

`rootfs-update` 要求目标根盘已经存在。新使用者尚无根盘时，按下一节使用 `rootfs-create` 创建；日常程序更新使用 `rootfs-update`。安装种子 `build/dev/fs.img` 随构建更新，已有持久根盘需要显式安装才会改变。

## 别人生成自己的 rootfs

通过 Git 获取的是源码和构建规则，不包含你的磁盘、备份或密码。
对方安装 README 中的工具链后，在项目根目录运行：

```sh
cmake --preset dev
cmake --build build/dev --target rootfs-create
cmake --build build/dev --target qemu
```

`rootfs-create` 构建安装种子并创建新的 `data/rootfs.img`。
它始终使用 fresh 模式：不导入旧盘；目标已存在就报错，不复用或覆盖别人的根盘。
新盘没有 passwd/shadow 等既有账户记录，也没有旧的用户主目录文件。
第一次启动会提示使用者设置自己的 root 密码，没有预设密码。
之后启动会重复使用这位使用者自己的根盘，密码和文件随之持久保留。

同一份源码也可以创建多份独立根盘：

```sh
cmake --preset dev -DXV_ROOTFS_IMAGE="$PWD/data/alice.img" -DXV_ROOTFS_IMPORT_IMAGE=
cmake --build build/dev --target rootfs-create
cmake --build build/dev --target qemu
```

需要切回原来的根盘时重新配置 XV_ROOTFS_IMAGE。已存在的磁盘不受创建新盘影响。
也可以直接指定输出路径，不改变 CMake 的默认启动选择：

```sh
python tools/create_rootfs.py --seed build/dev/fs.img --output data/another-owner.img --fresh
```

安装种子必须没有既有账户记录及用户主目录数据；误把个人根盘当成种子时会拒绝创建。
`rootfs-image` 是日常启动使用的幂等目标：根盘存在则保留，不存在则创建。
它与要求新建的 `rootfs-create` 不同。

## 显式迁移旧数据盘

`XV_ROOTFS_IMPORT_IMAGE` 新配置默认空，不再自动寻找 data/data.img。
有迁移需求时可用 create_rootfs.py 的 `--import-data` 或显式配置这个变量，
然后使用 `rootfs-image` 创建目标；`--fresh` 与 `--import-data` 不能同时使用。
旧的 CMake 构建目录可能仍缓存之前的导入路径，清空该变量即可禁用。
`rootfs-create` 不使用这个迁移选项，即使旧构建目录缓存了路径，也保持独立新建。
已有 rootfs 不会被迁移目标覆盖；源镜像需要离线，迁移细节见开发指南。

## 验证与数据位置

rootfs_update_contract 覆盖 dry-run、白名单、符号链接、硬链接、权限、容量不足、
发布失败、日志重放、备份、幂等及实际 QEMU 磁盘锁。
rootfs_update_1cpu / rootfs_update_5cpu 通过真实登录验证原密码、普通用户、
用户文件及新安装程序，另启动独立的新盘，让另一位使用者设置不同密码。
rootfs_image_contract 验证 fresh 拒绝复用和拒绝包含账户状态的种子。

测试只操作临时私有镜像。data 目录整体以及仓库内的 *.img 都被 Git 忽略，
涵盖根盘、备份和临时镜像；这些私人运行数据不属于源码发行内容。

## 名称与路径

系统与镜像工具的路径上限为 4096 字节（含 NUL）。ext3 的组件最多
255 字节，原 xv6 格式组件仍为 14 字节。rootfs-update 根据安装种子后端
检查清单名称；长名称程序可以安装到 /bin、/sbin 等允许的管理路径，
保护账户和个人目录的规则不变。把长名称种子安装到原 xv6 镜像时，无法
表示的组件会失败，原盘不会发布部分结果。内核和目录工具的规则见
[目录接口](directories.md)。
