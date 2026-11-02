# `kernel/io/fs_service.h`

`FS_SERVICE_EPOCH_SECONDS` 是原始块能力提供的 ioctl 请求号。参数指向用户态 `unsigned long`，内核复制当前实时时钟秒数，供后端生成磁盘时间戳。

该请求不解析路径，不读取文件系统格式，也不需要已经挂载根目录。原始盘能力的取得和生命周期见[引导块能力](../fs/file/bootstrap_block.cpp.md)。
