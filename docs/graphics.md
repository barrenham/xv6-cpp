# VirtIO-GPU 2D 显示

显卡后端属于 `xv.drivers:gpu` 内部分区，实现已有 `CharacterDevice` 和
`InterruptDevice`，通过板级装配、PLIC 路由和 devfs 登记接入系统。用户侧
使用 `/dev/fb0`（字符设备 29:0，root:root 0600），不暴露物理地址或内核指针。
普通无显卡启动仍可使用串口；此时 fb0 打开失败，驱动不分配显存。

## 运行

```sh
conda activate OS-LAB
cmake --build --preset dev -j6
cmake --build --preset dev --target rootfs-update
cmake --build --preset dev --target qemu-graphics
```

QEMU 使用 `virtio-gpu-device,bus=virtio-mmio-bus.3`，MMIO 地址
`0x10004000`，PLIC IRQ 4；磁盘、第二块磁盘和熵设备继续使用 slot 0/1/2。
`qemu-graphics` 打开 GTK 显示窗口，需要宿主图形会话。登录和命令输入仍在
串口终端进行。登录 root 后执行 `fbdemo`，窗口显示红、绿、蓝三个竖条。
无桌面时可以使用 `-display none` 和 QMP `screendump`，见 `tests/gpu.py`。

## 从 SSH 查看图像

使用 `qemu-vnc`，不需要服务器上的 GTK、X11 或桌面会话。VNC 默认监听
服务器的 `0.0.0.0:5901`，即所有 IPv4 接口。在自己电脑的 VNC 客户端中，
服务器填 **192.168.239.158**（或服务器的实际 IP），端口填 **5901**；
TigerVNC 可使用 `vncviewer 192.168.239.158::5901`。
也可以通过 SSH 隧道把它转到自己电脑的 15901 端口，步骤如下。
QEMU 的 VNC display 编号与端口关系是 `5900 + display`，依据
[QEMU 显示选项](https://www.qemu.org/docs/master/system/qemu-manpage.html)。

1. 在服务器的 SSH 终端进入仓库。如果已有 QEMU 使用同一根盘，先在客体
   执行 `sync`，再按 **Ctrl-a，然后 x** 退出旧 QEMU。启动 VNC 版本：

   ```sh
   cmake --build --preset dev --target qemu-vnc
   ```

   这个终端继续显示串口登录和 shell。登录 root 后执行 `fbdemo`。

2. 在**自己电脑**另开终端建立隧道，将 `user@server` 换成你的 SSH 地址：

   ```sh
   ssh -N -o ExitOnForwardFailure=yes -L 127.0.0.1:15901:127.0.0.1:5901 user@server
   ```

   如果平时 SSH 使用额外的 `-p`、`-i` 或跳板机选项，在这里同样使用。
   保持隧道终端开启。

3. 在自己电脑打开 VNC 客户端，服务器填 **127.0.0.1**、端口填 **15901**。
   若使用 TigerVNC 命令行，可写 `vncviewer 127.0.0.1::15901`，双冒号表示
   显式 TCP 端口，见 [TigerVNC 参数说明](https://tigervnc.org/doc/vncviewer.html)。连接后即可看到 `fbdemo` 的 RGB 色条。

初始登录和启动程序在第一个 SSH 串口终端进行；启动 fbterm 后，
可在 VNC 窗口直接输入，也可继续使用串口，见 [VirtIO 键盘](keyboard.md)。
运行 `fbterm` 可在 VNC 中显示交互 shell 的文字与命令结果，安装和操作见
[用户态图形终端](fbterm.md)。`fbterm --demo` 可单独演示文字和滚屏。
退出时在客体 `sync`，然后 Ctrl-a、x；停止隧道使用 Ctrl-c。

如服务器 5901 已被占用，可以在服务器改用 display 2（端口 5902）：

```sh
cmake --preset dev -DXV_VNC_DISPLAY=2
cmake --build --preset dev --target qemu-vnc
```

此时将隧道命令的最后一个 `5901` 改为 `5902`，本机端口 15901 可保持不变。
`qemu-vnc` 不会自动切换端口，启动时会打印实际配置的监听端口。

## 模块与控制协议

初始化通过 InitializeFlow 完成设备识别、modern VERSION_1 协商、DMA
候选分配、GET_DISPLAY_INFO、RESOURCE_CREATE_2D、ATTACH_BACKING、
SET_SCANOUT 和初次刷新。只选择首个启用的 scanout，启动配置尺寸（默认1280×800）。
磁盘/网络等其他模块无需了解 GPU 命令。协议依据
[VirtIO 1.2 GPU 章节](https://docs.oasis-open.org/virtio/virtio/v1.2/virtio-v1.2.html)。

帧缓冲采用 B8G8R8X8_UNORM（格式号2），整数像素为 `0x00RRGGBB`。
显存和attach请求按配置尺寸计算，GPU专用16槽队列支持多页请求。默认
1280×800使用1000个显存页和8个控制页；640×480仍为300+6页。
最大1920×1080使用2025+12页；设备缺失不分配显存。
配置方式、容量计算与终端自适应见 [分辨率说明](display-resolution.md)。

controlq 同时仅一条在途请求，descriptor 链区分设备只读请求与可写响应。
每条请求设置 FENCE，并校验完成索引增量、头 ID、响应长度、类型与 fence。
transfer 完成后才 flush。运行期使用 sleep 锁串行，等待时不持自旋锁或
关闭中断。当前采用最长 500 ms 的轮询，兼容尚无调度器和 IRQ 路由的启动
阶段；它不是异步高吞吐渲染接口。IRQ 处理器只确认中断和显示配置事件。

初始化分配失败、命令超时或协议错误均转为离线。释放 DMA 页面前先写设备
reset 并等待确认；若 reset 在 100 ms 内未确认，则保留页面所有权，防止
设备迟到 DMA 写入已回收内存。没有运行期热插拔或重新初始化入口。

## 用户接口

`kernel/io/framebuffer.h` 是项目自己的 v1 ABI，不是 Linux fbdev 二进制 ABI。

| ioctl | 参数 | 行为 |
| --- | --- | --- |
| XV_FB_GET_INFO | FramebufferInfo 的用户地址 | 返回版本、尺寸、行跨度、像素大小和格式 |
| XV_FB_DRAW | FramebufferUpdate 的用户地址 | 从用户像素缓冲复制一个矩形，transfer 后 flush |

DRAW 的 x/y/width/height 必须完全位于屏幕内，width/height 非零，stride
至少为 width×4，reserved 必须为零，源地址范围不得溢出。用户复制按行及
帧缓冲物理页边界拆分，验证地址访问权限；禁止把用户地址直接交给设备 DMA。
DRAW 还检查当前有效 UID 为 root，降权后不能利用继承的 fd 绘制。
所有参数和像素复制成功后才提交显示更新；复制失败可能改变已经复制的
backing 前缀，但不执行 transfer/flush，不提供整次更新的内存回滚保证。

原始 read/write 不提供像素流语义：零长度返回零，非零长度返回失败。
当前没有 mmap、硬件光标、VirGL/3D、运行期模式切换、内核显示终端或鼠标
驱动；用户态显示终端由 [fbterm](fbterm.md) 提供，
[VirtIO 键盘](keyboard.md) 与串口 TTY 均可提供输入。

## 验证

`driver_contract` 编译实际 GPU 模块，覆盖设备缺失、当前配置的全部DMA
分配失败点、跨多页backing请求、矩形/地址/ABI 检查、权限、错误 fence
和超时复位；同一契约使用 ASan/UBSan/泄漏检查运行。

`gpu_1cpu`、`gpu_5cpu` 运行真实 QEMU GPU 和用户程序，用 QMP 截取实际
配置尺寸的scanout，核对顶/中/底多处 RGB 像素。用户程序还检查坏用户地址、
非法矩形，以及 fork 后降权的继承描述符权限。`gpu_absent_1cpu` 检查无 GPU
时仍能启动和登录，并正确拒绝打开 fb0。
`gpu_vnc_5cpu` 使用无图形终端模式，其独立测试实例仍将 VNC 限定监听
127.0.0.1，通过真实 TCP 连接完成 RFB 握手，并核对收到的全部画面
像素与实际 scanout 一致。该测试不验证 `qemu-vnc` 目标的所有接口监听配置。

```sh
ctest --test-dir build/dev --output-on-failure -j2 -R '^(gpu_|driver_contract)'
```

截图与本地日志位于 `build/docs/gpu/`（不提交磁盘镜像或账户数据）。

本轮验收详见 [验收记录](graphics-acceptance.json)：15 项相关检查、真实显示截图、sanitizer 与两种模式共12个fork样本通过。此前109项全量检查是GPU接入前的历史证据，不计作本轮重新运行。
