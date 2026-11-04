# kernel/io/fs_control.h

通用 FS Control 载体 ABI 1：结构大小、载体/驱动版本、命令、输入/输出长度、命名空间和有界数据。
Readable/Writable 来自实际描述符，ReadOnly 来自挂载，均不由载体请求提供。无文件系统专用算法。
