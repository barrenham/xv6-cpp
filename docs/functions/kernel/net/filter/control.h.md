# `kernel/net/filter/control.h`

防火墙共享管理 ABI：版本、操作码、规则动作与固定容量。firewall_rule 为 64 字节，firewall_request 为 128 字节，不含指针；布局由 static_assert 检查。本文件没有可调用函数。
