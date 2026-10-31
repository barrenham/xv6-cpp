# 用户态图形终端 fbterm

`fbterm` 把现有 `/dev/fb0` 与命名 PTY 接起来。在帧缓冲上使用10×20字符单元，默认1280×800时提供128列、40行；
字符网格随启动分辨率自动调整，见 [分辨率配置](display-resolution.md)。无需修改内核，也不使用
mmap 或窗口系统。键盘输入来自 VNC 的 VirtIO 键盘及启动 QEMU 的 SSH/串口终端；
画面可以从 VNC 查看，PTY 输出也同步转发到串口。

## 安装与启动

在当前客体中执行 `sync`，再按 Ctrl-a、x 退出使用根盘的 QEMU。宿主机在
项目根目录运行：

```sh
conda activate OS-LAB
cmake --preset dev
cmake --build --preset dev -j6
cmake --build --preset dev --target rootfs-update
cmake --build --preset dev --target qemu-vnc
```

`rootfs-update` 会备份已有根盘并保留账户和用户文件。根盘被 QEMU 使用时
不能离线更新。新创建的根盘和测试镜像自动包含 `/bin/fbterm`。

VNC 客户端连接服务器实际 IP 的 5901 端口（当前 `qemu-vnc` 默认绑定
`0.0.0.0:5901`）。在串口以 root 登录后，执行：

```sh
fbterm --demo
fbterm
```

`--demo` 绘制超过一屏的文本，展示自动滚屏和绿色完成提示，然后返回串口
shell，画面保留。`fbterm` 默认创建 PTY 并启动一个独立的交互 shell。
在同一个 SSH/串口终端里输入：

```sh
ls /bin
ps
free
clear
```

命令及结果会同时出现在 VNC 和串口。方向键历史、退格、行编辑、管道与现有
shell 作业控制沿用原实现。Ctrl-C 传给内层前台任务；在空提示符按 Ctrl-D
或输入 `exit` 返回外层 shell。Ctrl-] 主动断开，挂断内层前台任务。
运行嵌套pty时，Ctrl-]先传给最内层代理，返回上层提示符后再按才继续退出；
换行与逐层退出规则见[嵌套PTY说明](nested-pty.md)。

默认会把命令输出复制到串口，这叫“串口镜像”。主要使用VNC时可以关闭它：

```sh
fbterm --no-serial-mirror
fbterm --no-serial-mirror cat /usr/share/doc/xv6/README
```

此时命令回显和结果只显示在屏幕中；串口输入和VNC键盘仍可使用，连接与退出
提示仍会写到串口。保持原SSH/串口终端打开，Ctrl-]仍可退出。普通 `fbterm`
继续保留镜像，既有脚本不必修改。选项放在命令前，`--`可结束选项解析。

也可以直接启动指定程序，例如 `fbterm cat`、`fbterm ps`。`fbterm --help`
显示用法。运行期间保持原 SSH/串口终端打开。图形启动目标已接入 VNC 键盘；
运行 fbterm 后点击 VNC 画面即可直接输入，详见 [键盘说明](keyboard.md)。

## 实现与边界

- `user/apps/fbterm/font.h` 是XV Mono 10×20灰度等宽字形表，基于DejaVu
  Sans Mono生成；字体授权随源码和镜像分发，见 [终端渲染](terminal-rendering.md)。
- `terminal.h` 管理字符与颜色网格、延迟自动折行、CR/LF、退格、制表、
  整屏滚动、固定下划线光标及跨读取保存的转义解析状态。
- 支持 shell 编辑和 `clear` 使用的 CSI 行/屏擦除、光标移动/定位、
  保存/恢复位置、16 色 SGR、粗亮色、反显、光标显隐；忽略 OSC 标题。
  这是有限的终端控制序列子集，`TERM=xv-fbterm`，不宣称完整 VT/xterm 兼容。
- 非 ASCII UTF-8 字符显示为 `?`；没有中文字体、历史滚屏查看、鼠标、
  剪贴板、光标闪烁或多窗口。清屏的 CSI 3J 因没有历史缓冲而忽略。
- 渲染把全部脏行合并为一次矩形提交；像素缓冲按实际屏幕分配，避免条带
  分别flush造成滚屏割裂。缓冲位于堆内存，1920×1080约7.91 MiB，不占用户栈。
- 使用 `/dev/ptmx` 创建 slave；将slave窗口设为实际字符网格与像素尺寸，
  启用 OPOST/ONLCR。子进程创建会话、取得控制终端并将 slave 复制到 0/1/2。
  子程序继承当前目录、账户身份和环境，并设置 TERM、COLUMNS、LINES。
- 代理使用 poll 和非阻塞 I/O，保留部分输入/输出及 EAGAIN 后的尾部。
  串口采用 raw 模式以转发信号控制字节；退出恢复 termios 与共享文件标志。
  正常 EOF、主动断开、I/O 错误和可捕获的 HUP/TERM/INT 均走清理路径。
  SIGKILL 无法运行用户态恢复逻辑，避免强杀代理。
- root 权限与设备访问约束沿用 GPU ABI。需要受支持尺寸的BGRX8888模式。
  同时只运行一个绘制程序，多个 `/dev/fb0` 使用者之间没有画面仲裁。

## 验证

```sh
cmake --build --preset dev --target test-image -j6
ctest --test-dir build/dev --output-on-failure -j2 -R '^fbterm_'
```

`fbterm_contract` 在宿主运行实际终端模型，验证字形像素、折行、滚动、
分段转义、shell 重绘、颜色、光标、超长参数和确定性随机字节流。可单独
启用 AddressSanitizer/UndefinedBehaviorSanitizer：

```sh
"$CONDA_PREFIX/bin/clang++" -std=c++20 -O1 -g -fsanitize=address,undefined \
  -I. tests/fbterm_contract.cpp -o build/dev/fbterm_contract_sanitized
build/dev/fbterm_contract_sanitized
```

`fbterm_1cpu`/`fbterm_5cpu` 从独立磁盘启动真实 QEMU，使用 QMP 截图解码
实际字符网格，检查 demo 滚屏、颜色以及 shell 中的 ls/ps/free、清屏和
退格；同时验证大于转发缓冲的完整输出、PTY 输入、Ctrl-C、EOF、运行中
raw 任务的主动断开、原串口模式与文件标志恢复、exec 失败。独立
`fbterm_absent_1cpu` 检查无 GPU 时退出并保留可用 shell。

本地截图和串口日志写入 `build/dev/fbterm-*.ppm`/`fbterm-*.log`。
验收范围见 [fbterm 验收记录](fbterm-acceptance.json)。
