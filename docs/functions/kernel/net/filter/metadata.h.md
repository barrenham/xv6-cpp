# `kernel/net/filter/metadata.h`

解析器与规则引擎共享的 Metadata 值对象，记录地址、协议、端口、入/出接口、IPv4 总长和端口是否可用，不含报文指针，不依赖管理 ABI、规则存储、驱动或锁。本文件没有可调用函数。
