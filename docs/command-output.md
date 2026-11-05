# 常用命令的输出格式

管理工具默认面向交互阅读：有表头、数字列右对齐、名称列左对齐，大小标注单位，状态使用文字。排版只使用 ASCII 与空格，不依赖 Tab、颜色或终端转义，适用于串口与 fbterm。

## drvctl

`drvctl list` 显示已加载的驱动模块，`drvctl devices` 显示内核发现的设备。两张表中的编号用途不同：

| 列 | 含义 |
| --- | --- |
| MODULE | 已加载模块编号，作为 drvctl bind 的第一个参数 |
| DRIVER | 驱动模块名称 |
| DEVICES | 该模块已绑定的设备数量，0 表示尚未绑定 |
| TRANSFERS | 成功的块设备读写请求数或网卡发送请求数，不是字节数；其他驱动目前未统计此项，0 不代表设备没有工作 |
| INTERRUPTS | 已处理的硬件中断次数 |
| ABI | 驱动与内核之间的接口版本 |
| DEVICE | 设备枚举编号，作为 drvctl bind 的第二个参数 |
| NAME | 设备资源名称，例如 virtio-mmio.0、uart0 |
| BUS | virtio-mmio、platform、software 或 pci |
| DEVICE_ID | 总线中的设备类型标识，以十六进制显示；不是绑定参数 |
| IRQ | 硬件中断号，`-` 表示没有硬件中断 |
| STATE | BOUND 表示已有驱动，UNBOUND 表示尚未绑定 |

活动计数从模块加载后累积。绑定命令为 `drvctl bind MODULE DEVICE`；已经绑定的设备不能重复绑定。列表末尾也会输出主要字段说明。

脚本需要原有键值格式时使用：

```sh
drvctl --raw list
drvctl --raw devices
```

## ls、mount 与 fsctl

- `ls`：TYPE、INODE、SIZE(B)、NAME；文件类型显示为 file、directory、char-dev、block-dev、symlink 等文字，大小使用 64 位字节计数，名称放在最后一列并完整保留。
- `ls -l`：MODE、LINKS、UID、GID、SIZE(B)、NAME；分别是权限、硬链接数、属主用户编号、属组编号、字节大小和名称。多个路径分组显示。
- `mount`：FILESYSTEM、MOUNTPOINT；`mount --raw` 保留旧的 `类型 on 挂载点` 格式。
- `fsctl PATH`：逐行显示路径、FS 扩展接口名、接口版本、桥接协议版本、读写模式、最大载荷字节数、快照能力和支持的操作名称。接口名与具体 FS 类型有关，可能为空。
- `fsctl PATH INTERFACE VERSION COMMAND [HEX]`：默认将回复按每行 16 字节分组，以十六进制显示偏移与数据；`fsctl --raw ...` 保留旧输出，`-w` 仍用于可写打开。

ps 与 free 已使用带表头、列宽及 KiB 单位的格式，本轮保持其输出。netctl、iptables 等其他管理命令仍使用各自的展示方式。
