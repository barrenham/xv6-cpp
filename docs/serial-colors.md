# 串口终端配色

统一配色已扩展到全部应用的提示、帮助、错误和管理界面，详见 [用户程序终端样式](user-terminal-style.md)。本页保留 shell 与 ls 的具体配色和控制选项。

串口 shell 和 `ls` 使用 ANSI SGR 序列，由连接串口的终端模拟器显示颜色。无需 GPU、VNC 或额外串口驱动；终端背景、字体和具体色值仍由客户端配置。

| 内容 | 默认颜色 |
| --- | --- |
| root 提示符 | 暖黄色 |
| 普通用户提示符 | 亮绿色 |
| `ls` 表头 | 默认文字色加粗 |
| 目录 | 亮蓝色 |
| 带执行权限的普通文件 | 亮绿色 |
| 符号链接 | 紫色 |
| 字符/块设备 | 亮黄色 |
| socket | 亮紫色 |
| 普通文件、数据列、输入的命令 | 客户端默认前景色 |

启动并登录后，提示符自动配色，直接运行 `ls /`、`ls /bin`、`ls -l /dev` 即可查看。每个着色字段和提示符结束后恢复默认属性，不给后续命令输入遗留颜色。行编辑器计算提示符宽度时只计可见字符，重绘、换行、方向键和历史记录仍使用相同的列位置。

## 控制颜色

```sh
ls --color=auto /bin    # 默认：仅在 stdout 为终端时配色
ls --color=never /bin   # 这一次不配色
ls --color=always /bin  # 强制配色，也可以写成 --color
ls -l --color=auto /dev

export NO_COLOR=1       # 关闭 shell 提示符和 ls 的自动配色
unset NO_COLOR         # 恢复自动配色
```

`TERM=dumb` 也会关闭自动配色。未设置 `TERM` 时仍对串口终端启用颜色。非空 `NO_COLOR` 关闭自动配色；显式 `ls --color=always` 优先于这些环境变量。

自动模式检查输出描述符：`ls` 检查 stdout，shell 提示符检查 stderr。`ls /bin > /tmp/list` 和 `ls /bin | cat` 生成纯文本，`echo input | ls /bin` 的输出仍可配色。去掉颜色序列后，列表内容、列宽和权限信息与纯文本模式相同。

## 实现和验证

`user/runtime/terminal_style.h` 提供环境变量和目标终端检测；`user/apps/filesystem/ls/style.h` 管理文件类型配色。shell 保留纯文本提示符供行编辑器计宽，输出和重绘通过 `prompt_display` 加入颜色。表格仍按行批量提交，不恢复逐字符 write。

`serial_colors_1cpu` 和 `serial_colors_5cpu` 在独立 QEMU 测试盘上通过实际 UART 验证颜色、颜色重置、文件类型、纯文本等价性、管道/重定向、环境变量和跨 80 列边界的光标编辑。测试日志保存到构建目录的 `serial-colors-{1,5}cpu.log`。

2026-11-08 验证：两项串口配色测试以及 `session_5cpu`、`workflow_1cpu` 共 4/4 通过，汇总日志为 `build/serial-colors-validation.log`。持久根盘已安装新的 `/bin/sh` 和 `/bin/ls`，可重新启动 QEMU 查看。
