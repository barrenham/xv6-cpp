# `kernel/fs/image_paths.h`

源码：[启动与命令路径常量](../../../../kernel/fs/image_paths.h)。

集中定义镜像安装约定：初始程序 `/sbin/init`、默认 shell `/bin/sh`、命令搜索路径 `/bin:/sbin:/usr/tests` 和文档 `/usr/share/doc/xv6/README`。内核首次装载、init、账户工具与运行库复用这些常量；安装映射由 CMake 生成的 `fs-manifest.tsv` 明确指定。

本文件只定义路径宏，没有独立函数。修改布局时同步检查镜像清单、启动入口、PATH、账户默认 shell，以及依赖程序或文档的测试路径。

## 函数导航



无独立函数项。
