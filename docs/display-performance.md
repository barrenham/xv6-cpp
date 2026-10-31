# 图形终端性能排查

2026-11-01，在当前1920×1080配置下使用独立测试镜像。排查阶段未更新主系统镜像。

本页保留优化前数据；后续实现和复测见[串口镜像与批量输出优化](terminal-output-optimization.md)。

## 结论

本地连续文本输出的主要瓶颈是 **串口镜像与PTY读取串行耦合**，非阻塞TTY
逐字节发送放大其开销。五核正常模式中，串口write调用区间约占传输循环的
90.4%；仅关闭串口镜像、保留图形绘制和GPU提交，循环中位耗时从22.58s降至
1.32s。增加本地VNC客户端持续读屏后，结果接近未启用VNC的对照。

这确定了当前测试负载的优先优化路径，不能据此排除实际MobaXterm、网络或
其他输出模式的额外开销。未测量远程网络延迟或主观动画流畅度。

## 测量方法

`tools/profile_fbterm.py` 为每次QEMU启动复制fs-test.img，在副本中创建500行
ASCII文本。经过PTY的输出为22013字节，使用 `fbterm cat /burst.txt`，不依赖
curl、DNS或外网。每种模式运行两轮，第二轮反转模式顺序；同一虚拟机内运行
各模式，单核与五核测试依次进行。没有修改刷新策略，也未运行fork门禁。

`FBTERM_PROFILE` 仅在显式设置时启用计时/消融实验：

| 值 | 实验路径 |
| --- | --- |
| normal | 正常解析、绘制、GPU提交、串口镜像，增加计时 |
| no-mirror | 保留解析与完整图形显示，跳过串口镜像，串口输入仍可用 |
| no-present | 保留解析、栅格化与串口镜像，跳过XV_FB_DRAW |
| no-paint | 保留解析与串口镜像，跳过栅格化和GPU提交 |
| parse-only | 只接收、解析输出，跳过绘制和串口镜像 |

所有模式核对解析字节数；有镜像的模式额外逐字节核对完整串口文本，不能通过
丢失输出获得加速。no-mirror仍执行相同的栅格化和真实GPU ioctl；本工具不做
GPU像素OCR，最终像素正确性由正常路径的fbterm回归覆盖。

计时范围是transfer循环，包含EOF前最终绘制，不包含初始清屏、fork/exec、
终端配置及诊断报告输出。时间使用clockticks的10MHz基准，是阶段经过时间，
包含调度延迟，**不是线程纯CPU时间**。每阶段额外读时钟会带来少量开销。
工具同时记录宿主端完整命令经过时间与QEMU进程CPU时间，两者范围更广；
多核QEMU的CPU秒数可以超过墙钟秒数。

## 两轮中位数

下表为transfer循环秒数；消融改变吞吐、帧数和调度，因此不同实验的总时间
不能简单相减作为某个阶段的精确独立成本。

| 模式 | 单核，无VNC | 五核，无VNC |
| --- | ---: | ---: |
| normal | 6.79 | 22.58 |
| no-mirror | 1.24 | 1.32 |
| no-present | 5.05 | 19.48 |
| no-paint | 3.31 | 16.15 |
| parse-only | 0.87 | 0.40 |

五核normal的阶段中位数：串口写入20.41s，XV_FB_DRAW 1.23s，栅格化0.63s，
文本处理0.10s，PTY读取0.10s，poll 0.07s。百分比取各轮阶段占比的中位数：
串口90.4%、draw 5.5%、栅格化2.8%。未把内核像素复制和GPU等待进一步拆分，
它们均包含在draw中；当前证据不支持将GPU忙等判为首要瓶颈。

五核no-mirror只需约3帧，而normal约11帧；串口延长整段输出，也增加了绘制
中间画面的次数。normal每轮向驱动提交约90MB像素，no-mirror约22～24MB。
此处是用户态到驱动的像素量，**不是VNC网络传输量**。

五核VNC对照使用本机TCP连接，完成RFB握手，选择Raw编码并持续发送增量
读屏请求；每轮normal实际收到约12～13次画面更新，客户端不是闲置连接。

| 五核模式 | 无VNC | 有活跃本地VNC客户端 |
| --- | ---: | ---: |
| normal | 22.58s | 21.70s |
| no-mirror | 1.32s | 1.18s |

两轮数据存在调度波动；该小差值不能解释为VNC带来加速。它表明本次本地
负载中，VNC不是占主导的额外成本。Raw客户端不等同于MobaXterm的编码设置。

## 对应代码

- `user/apps/fbterm/fbterm.cpp`：仅在sent==used时读PTY；每批内容必须先完成
  串口镜像，未写完前禁止继续读取PTY。所有这些操作与绘制共用同一个循环。
- `kernel/drivers/tty/core/tty.cpp`：WriteFlow::read在nonblocking时设置part=1。
  一次较大的write会对每个字节重复copy_in、输出转换和transmit。
- `kernel/drivers/serial/flow.cppm`：每次transmit均走写者锁、状态锁及pump路径；
  `uart16550.cpp`中的pump还会执行设备寄存器读写及进度通知。
- `kernel/drivers/display/virtio_gpu.cpp`：像素复制后同步transfer/flush，完成采用
  有界轮询。该开销仍存在，但优先级低于当前串口路径。

五核下串口路径明显慢于单核。逐字节锁、通知和MMIO是代码中存在的放大因素，
但本轮没有分别计数锁竞争、通知次数或设备等待时间，不能给出它们各自占比。
不能仅凭五核结果认定调度器或全系统SMP实现有错误。

## 优化顺序

1. 为正常使用增加明确的串口镜像开关；VNC工作时可关闭镜像并保留串口输入。
2. 优化非阻塞TTY批量发送，保留短写、EAGAIN、信号以及ONLCR转换的字节语义。
3. 镜像需要保持时，用有界队列解除短时突发的耦合。持续输出高于串口能力时，
   有限队列仍会产生背压，不能靠无限加缓冲解决。
4. 再测滚屏像素复用、显示提交和GPU完成等待，决定是否实现异步提交。

本轮只加入可关闭的诊断路径和工具，没有实施以上行为改造。
诊断未启用时的fbterm_contract和fbterm_1cpu回归均通过；后者包含2000行
完整输出、实际GPU像素、PTY输入/信号、EOF/分离和终端状态恢复。

## 复现

```sh
conda activate OS-LAB
cmake --build --preset dev
python3 tools/profile_fbterm.py --cpus 1 --rounds 2 \
  --output build/docs/display-profile/1cpu.json
python3 tools/profile_fbterm.py --cpus 5 --rounds 2 \
  --output build/docs/display-profile/5cpu.json
python3 tools/profile_fbterm.py --cpus 5 --rounds 2 --vnc-client \
  --modes normal no-mirror --output build/docs/display-profile/5cpu-vnc.json
```

上述命令只改测试副本；不会连接或停止用户自己的QEMU。诊断环境变量只用于
排查，no-present/no-paint/parse-only会故意停用部分显示功能。

[机器可读结果](display-performance-results.json)保存全部24轮原始计时及模式、
分辨率、字节数和VNC流量。完整串口日志及默认路径回归日志位于
`build/docs/display-profile/`。
