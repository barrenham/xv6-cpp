# VirtIO 键盘与 VNC 输入

`qemu-vnc` 和 `qemu-graphics` 现在同时装配 VirtIO-GPU 与
`virtio-keyboard-device`。键盘占用 VirtIO MMIO slot 4：`0x10005000`，
PLIC IRQ 5。无图形的 `qemu` 仍可单独使用串口，无键盘时不分配键盘 DMA 页。

## 使用

更新用户程序前，先在当前客体执行 `sync`，按 Ctrl-a、x 退出使用根盘的
QEMU，然后在宿主项目目录执行：

```sh
conda activate OS-LAB
cmake --preset dev
cmake --build --preset dev -j6
cmake --build --preset dev --target rootfs-update
cmake --build --preset dev --target qemu-vnc
```

VNC 客户端连接服务器实际 IP 的 5901 端口。先在 SSH 串口登录 root 并
运行 `fbterm`；看到 `VNC and serial keyboard` 后，点击 VNC 画面，即可在
VNC 窗口中输入 `ls`、`ps`、`free` 等命令。SSH 串口也继续接受输入。

支持 US 主键区的字母、数字、标点，左右 Shift/Ctrl/Alt、Caps Lock、
回车、退格、Tab、Esc、方向键、Home/End、Insert/Delete、PageUp/PageDown。
修饰键在用户态处理，Ctrl-C/Ctrl-D/Ctrl-Z 等仍由 PTY 和 shell 处理。
Ctrl-] 可主动退出 fbterm，原串口状态在退出时恢复。

当前图形键盘只在 fbterm 打开 `/dev/kbd0` 期间送入内层 PTY。VNC 不提供
启动阶段的图形登录；初始登录与启动 fbterm 仍从串口完成。不提供鼠标、
中文输入法、完整国际键盘布局、功能键、数字小键盘数字或 LED 状态反馈。

## 内核接口与所有权

- `kernel/drivers/input/virtio_keyboard.cppm/.cpp` 是 `xv.drivers:keyboard`
  内部分区，实现现有 `CharacterDevice` 与 `InterruptDevice`。
- `/dev/kbd0` 是字符设备 `30:0`、root:root `0600`。每次只允许一个打开
  文件持有输入；dup/fork 引用共享打开文件，最后一个引用关闭时释放占用。
  read/poll 再检查当前有效 UID，降权继承描述符不能读取键盘事件。
- `kernel/io/keyboard.h` 定义项目 ABI `KeyboardEvent`：16 位 type、16 位
  code、32 位 value，共 8 字节；不是 Linux 带时间戳的 `input_event`。
  读取长度须为 8 的倍数，单次最多返回32个事件；零长度读取返回零。
- 支持阻塞可中断 read、O_NONBLOCK 的直接 `-EAGAIN` 和 poll。用户复制
  失败不消费事件。打开前或关闭后的输入被丢弃，重开不重放旧事件。
- 软件缓冲最多256个事件；溢出清除旧队列并先报告 SYN_DROPPED，fbterm
  收到后清除修饰键状态。过载时不能保证保留按键或仍按住的修饰键状态。
- 初始化协商 modern VERSION_1，查询 EV_KEY 的 A 和 Enter 能力，区分
  键盘与其它 VirtIO 输入设备。eventq 和 statusq 均设置为64槽，statusq
  保持空闲；总计7个DMA页，28 KiB。事件缓冲只位于内核拥有的 DMA 页。
- IRQ 在 spin 锁内 ACK，检查 used 索引增量、描述符ID、精确8字节长度与
  批内唯一性；保存验证后的ID，完成有界复制及描述符回填，然后唤醒读者。
  协议错误使设备离线并保留 DMA 页面，IRQ 不等待设备复位。
- 初始化失败先复位；仅确认复位后回收候选DMA页，100ms未确认则保留。
  当前没有热插拔、运行期重新初始化、按键状态快照或 LED 写入接口。

## 用户态接入

`fbterm` 使用 poll 同时等待串口、键盘、PTY 与串口输出。键盘事件经
`user/apps/fbterm/keyboard.h` 转换成字符或转义序列后，复用现有输入缓冲
和 PTY 写入背压路径。键盘离线时关闭键盘描述符，串口交互保留。

`display` 驱动已展开原有密集语句，使用显式分支块和具名 GPU 命令、
状态、fence 常量；GPU ABI、显存布局、FENCE 验证和 DMA 回收协议保持。

## 验证

```sh
cmake --build --preset dev --target kernel test-image -j6
ctest --test-dir build/dev --output-on-failure -j2 \
  -R '^(keyboard_[15]cpu|driver_contract|fbterm_contract)$'
```

`keyboard_1cpu`/`keyboard_5cpu` 完成真实 RFB 3.8 握手并发送 VNC KeyEvent，
验证字符、Shift/Caps Lock、退格、历史方向键、cat 输入和 Ctrl-C；QMP
截图解码确认输入结果确实出现在 GPU scanout。实例使用独立测试磁盘与
临时 loopback VNC 端口，不占用实际根盘和5901。

`driver_contract` 使用实际键盘模块，覆盖设备缺失、7个DMA分配失败点、
VERSION_1/能力检查、独占与降权权限、poll/EAGAIN、阻塞唤醒、复制失败、
缓冲溢出、65536索引回绕，以及非法完成ID/长度/增量/重复描述符。
`fbterm_contract` 覆盖双Shift、Caps重复、Ctrl组合、同步丢失和方向键转换。
两者另使用ASan/UBSan及泄漏检查运行。

协议依据：[VirtIO 1.2 输入设备章节](https://docs.oasis-open.org/virtio/virtio/v1.2/virtio-v1.2.html)。
本轮结果与实际安装状态见 [键盘验收记录](keyboard-acceptance.json)。

图形分辨率默认1280×800，最高1920×1080；输入路径不依赖固定行列，见 [分辨率配置](display-resolution.md)。
