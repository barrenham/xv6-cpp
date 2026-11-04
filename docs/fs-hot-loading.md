# 在运行中的内核上加载新文件系统 ELF

内核先启动。新文件系统的源码、ELF 和清单可以在启动后创建、编译和传入。
挂载管理器每次读取 `/etc/fsplugins/*.conf`，启动清单指定的用户态 ELF，并把它注册到通用 VFS 桥接层。
新增驱动不需要重启系统，也不需要重新编译内核或引导程序；类型名不需要预先登记在它们的源码里。

默认引导包只保留启动所需的五种后端。ext4 是单独构建的可选驱动，默认既不放入引导包，也不预装到系统盘。

## 系统运行时，在宿主机编译

下面的操作可以在 QEMU 已经进入 shell 后执行。SDK 导出工具复制用户态 SDK、运行库和 ABI 头文件，
不复制或编译内核实现。独立 CMake 项目没有 kernel/fsboot 构建目标。

```sh
python3 tools/export_fs_driver_kit.py /tmp/newfs-kit
cmake -S /tmp/newfs-kit/examples/fs-driver -B /tmp/newfs-build -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=/tmp/newfs-kit/examples/fs-driver/toolchain.cmake \
  -DXVFS_KIT_ROOT=/tmp/newfs-kit -DXVFS_DRIVER_NAME=hotext4
cmake --build /tmp/newfs-build -j 1
```

产物是 `hotext4.elf` 与 `hotext4.conf`。默认例子实现 ext4 的只读解析；更换成自己的驱动时，配置：

- `XVFS_DRIVER_INTERFACE`：自己的 C++ 模块接口。
- `XVFS_DRIVER_IMPLEMENTATIONS`：自己的源文件列表。
- `XVFS_DRIVER_ENTRY`：实现插件启动契约的 main。
- `XVFS_DRIVER_NAME`：新的文件系统类型名。
- `XVFS_DRIVER_RESOURCE` 与 `XVFS_DRIVER_SOURCE`：所需资源类别和默认源。

也可完全自己构建 ELF，只使用公共 C ABI；不要求继承此示例或使用项目的 C++ 模块。
纯 C 示例见 `examples/fs-plugin/`，入口与资源规范见 [插件开发规范](fs-plugin-authoring.md)。

## 传入正在运行的系统

以本项目已验证的 QEMU user 网络为例，在宿主机另一个终端启动本地文件服务：

```sh
python3 -m http.server --bind 127.0.0.1 --directory /tmp/newfs-build 8000
```

来宾通过 `10.0.2.2` 访问这个宿主机服务。使用其他网络配置时换成来宾实际可达的地址。
在来宾 root shell 中执行：

```sh
curl -fsS -o /usr/lib/fsplugins/hotext4.tmp http://10.0.2.2:8000/hotext4.elf
chmod 0755 /usr/lib/fsplugins/hotext4.tmp
mv /usr/lib/fsplugins/hotext4.tmp /usr/lib/fsplugins/hotext4.elf
curl -fsS -o /etc/fsplugins/hotext4.new http://10.0.2.2:8000/hotext4.conf
mv /etc/fsplugins/hotext4.new /etc/fsplugins/hotext4.conf
mkdir /tmp/newfs
mount -t hotext4 /tmp/newfs
```

清单最后发布，管理器就不会在 ELF 尚未传完时加载它。这里 `source=1` 指 QEMU 启动时已接入的第二块磁盘。
新驱动的加载与新物理磁盘的热插拔是两件事；本例不需要在运行时增加硬件。
可以用 `mount -t hotext4 -s SOURCE /tmp/newfs` 覆盖默认源，资源解释由清单指定。

挂载成功后，普通 ls/cat/open/read/exec 都经过新 ELF。`fsctl /tmp/newfs` 查看驱动声明的原生接口。
`umount /tmp/newfs` 在没有打开引用时卸载，该实例退出；以后再次挂载可启动同一 ELF 的新实例。
更新已有驱动时先卸载所有旧实例，再替换 ELF 与清单。

## 明确的边界

运行时加载不会自动生成文件系统算法。ELF 必须真正实现所声明的功能，驱动与公共 ABI 必须兼容。
驱动以用户态服务进程运行；管理员负责安装可受信任的 ELF。当前入口需要 root 和在线挂载管理器。
系统盘必须允许写入插件及清单目录；工具不会代替用户重建或清空根盘。
ext4 示例目前只读，其格式支持范围见 [ext4 驱动](ext4.md)。

验收 `fs_hotplug_1cpu` / `fs_hotplug_5cpu` 严格检查：启动前无 ext4 清单、ELF 或引导资产；
进入 shell 后才导出源码并编译新 ELF；经来宾 curl 实时安装、挂载、读取、执行、卸载和重新挂载；
同一个 QEMU 进程只有一次内核启动，内核/引导包哈希不变，传入 ELF 字节与编译产物完全一致。


## 本次实际验收（2026-11-04）

最终 14 项检查全部通过（313.92 秒）。其中启动后独立编译并热加载的两项分别为：
单核 36.59 秒、五核 43.91 秒。全过程保持同一个 QEMU 进程、一次内核启动，内核、引导包和原测试镜像哈希不变；
新 ELF 在首次 shell 提示符之后才编译完成，安装后从磁盘读回的字节与编译产物完全一致。

另四项 ext4 QEMU 检查在运行中传入驱动后挂载，覆盖 1/4 KiB 数据盘和 1/5 核；
原生契约测试覆盖五种布局、元数据损坏与结构拒绝。ext3/xv6 启动和独立根插件引导检查也通过。
日志见 `build/fs-live-driver-acceptance.log` 与 `build/dev/hot-filesystem-{1,5}cpu.log`。
测试仅使用私有磁盘副本，没有安装到真实 `data/rootfs.img`，没有修改实际账户。
