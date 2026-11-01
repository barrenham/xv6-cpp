# 输出处理与显示解耦、复制成本拆测

2026-11-01。保留1920×1080、1024行历史、串口镜像选项和原有刷新预算。

## 两个进程，一份在途快照

正常fbterm启动显示进程。主进程处理PTY、串口/VNC输入和历史；显示进程独立
栅格化、调用GPU并等待完成。主进程通过非阻塞管道发送可见字符快照，不发送
约7.91 MiB的像素数组。快照去掉默认空白行尾，普通文字按字符发送，颜色仅在
变化时编码；接收端校验长度、行宽、颜色和字符。进程间没有共享可变字符网格。

只有一份快照可处于发送/绘制/等待确认状态。期间主进程继续处理到达的字节，
保留最新终端状态；完成后按FramePacer预算发送最新状态，不排队显示过时的
中间帧。原有100ms批量合帧和按绘制成本设置间隔的规则保持，完成反馈不会
清除等待显示的新输出。主循环每次处理有界PTY批次，并继续服务键盘/滚轮。

EOF或Ctrl-]退出时，先等在途帧完成，再显示最后的状态；正常等待有5秒上限，
失败返回I/O错误并回收显示进程。显示进程关闭继承的PTY主端和串口描述符，
不延长PTY存活时间；原有嵌套pty退出及终端恢复行为保留。

这会增加一个fbterm进程和可能的COW像素副本，未提供线程或共享framebuffer。
单核也能在显示进程等待设备时运行主进程；CPU密集栅格化仍需要调度时间。
开启串口镜像时仍须完整发送每个字节，持续慢串口产生的背压保持。

## 减少搬运，定位主要成本

Renderer在前后两份字符快照中计算所有非空文字、非默认背景及光标的最右
边界。复用像素时只搬运这部分宽度，提交矩形也裁到这个边界；旧内容包含在
边界内，保证短行覆盖长行、清屏、彩色背景和光标移动不会遗留尾部。初始帧、
尺寸/代际变化和提交失败后的重绘仍覆盖整屏。满宽文字或背景仍可能提交整屏。

分段计时发现内核copy占据测试中的大部分DRAW耗时。底层runtime::move原来
逐字节复制；现在双方地址同余对齐时使用may_alias unsigned long机器字搬运，
首尾逐字节处理。向后重叠倒序，其他情况正序；不同余对齐回退字节路径。
页表访问权限与逐页用户复制保持。该函数是通用内核原语，也影响其他复制路径。

这一轮使用已有安全复制接口降低成本，没有引入共享framebuffer、GPU滚屏或
异步内核GPU队列。GPU transfer/flush依然同步，但在独立显示进程中执行。

## 计时与复现

诊断不默认打印实时日志，退出时输出，避免日志干扰显示。

```sh
env FBTERM_PROFILE=frames fbterm --no-serial-mirror
env FBTERM_PROFILE=frames fbterm --benchmark-scroll
```

- `FBPROFILE`：原有阶段和逐帧统计；`busybytes`为有快照在途时继续解析的字节。
  该窗口包括管道发送、绘制及设备等待，不能解释为仅GPU等待期间的字节。
- `FBSTAGES move`：用户态复用像素搬运的10MHz ticks，是raster的子集。
  raster还包含变化比较、填充、字形绘制和状态更新；不能把两项相加。
- `FBSTAGES copy_ms / transfer_ms / flush_ms`：内核三阶段累计墙钟毫秒。
- `copied / gpudraws`：完整成功复制的字节和次数，GPU提交失败前的成功复制
  也会计数；失败复制仍累计耗时，但不计部分字节。
- 原`XV_FB_GET_STATS`保持32字节。新`XV_FB_GET_TIMING`使用独立40字节ABI，
  需root；统计按设备累计，测试取开始/结束差值。计时包含调度和设备等待，
  不代表CPU忙碌时间，毫秒计时还有单次舍入误差。

仅启用诊断时可用`FBTERM_DISPLAY=sync`恢复同进程绘制对照，或用
`FBTERM_DAMAGE=full`保持整屏宽度；普通启动默认异步及裁剪。full仍复用像素，
与`FBTERM_RASTER=full`的完整字体重绘参考不同。

```sh
python3 tools/profile_fbterm.py --cpus 5 --rounds 2 --benchmark-scroll \
  --damage full --output build/docs/display-pipeline/word-full.json
python3 tools/profile_fbterm.py --cpus 5 --rounds 2 --benchmark-scroll \
  --output build/docs/display-pipeline/word-trimmed.json
python3 tools/profile_fbterm.py --cpus 5 --rounds 2 --lines 2000 \
  --no-serial-mirror --modes frames --display sync \
  --output build/docs/display-pipeline/final-sync-burst.json
python3 tools/profile_fbterm.py --cpus 5 --rounds 2 --lines 2000 \
  --no-serial-mirror --modes frames \
  --output build/docs/display-pipeline/final-async-burst.json
```

工具使用私有镜像，检查完整PTY字节数；有镜像时再检查全部串口文本。固定
滚屏测试绕过PTY和节流，不能用来衡量异步输出收益。同步/异步输出对照使用
同一优化后的驱动和渲染器，输出速度不同可能改变帧数，不能只比较单帧耗时。

## 本次测量

五核、1920×1080，每组两轮，每轮60次固定单行滚屏。各组QEMU依次运行，
没有并行回归客体。基线使用同一GPU计时实现和优化前的runtime::move；
裁剪对照保留相同的像素复用算法，未换成完整字体重绘。

| 指标 | 逐字节复制、整宽 | 机器字复制、整宽 | 机器字复制、裁剪 |
| --- | ---: | ---: | ---: |
| 每帧内核copy平均 | 90.23 ms | 22.18 ms | 11.27 ms |
| 每帧transfer平均 | 4.24 ms | 3.70 ms | 4.40 ms |
| 每帧flush平均 | 1.84 ms | 1.59 ms | 1.68 ms |
| 每帧用户像素搬运平均 | 27.72 ms | 23.50 ms | 13.88 ms |
| DRAW单帧中位数 | 97.94 ms | 25.87 ms | 17.17 ms |
| 每轮像素提交量 | 497.66 MB | 497.66 MB | 127.01 MB |
| 60帧循环耗时中位数 | 8.08 s | 3.65 s | 2.54 s |
| 整条命令QEMU CPU秒中位数 | 17.85 s | 12.56 s | 11.72 s |

copy/transfer/flush和像素搬运为累计阶段时间除以帧数，DRAW为全部120帧的
中位数；不能把这些不同统计量直接相加。基线两轮循环分别10.13s、6.03s，
波动明显；仅两轮的结果不能承诺所有负载都有相同收益。GPU两阶段没有随裁剪
显著下降，此负载中主要收益来自减少CPU复制。QEMU CPU秒覆盖完整命令，
也包含初始画面及报告，不等于循环内线程CPU时间。

另一组2000行PTY输出（88013字节，关闭串口镜像）使用同一优化后的内核和
裁剪渲染器，比较显示并行化：

| 指标 | 同步显示 | 异步显示 |
| --- | ---: | ---: |
| 循环耗时中位数 | 1.458 s | 1.343 s |
| 每轮提交帧数 | 8 / 7 | 4 / 4 |
| 每轮提交像素 | 14.52 / 12.70 MB | 7.26 / 7.26 MB |
| 在途期间继续解析的字节 | 0 / 0 | 45613 / 42434 |
| 完整命令QEMU CPU秒中位数 | 4.64 s | 4.56 s |

两轮均解析完整88013字节。异步主要减少中间帧并解除同步等待阻塞，不能据此
声称CPU使用率大幅下降。另做一轮活跃本地Raw VNC：完整输出、4帧、循环
1.253s，未发生协议/设备错误；这不是远程MobaXterm体验的等价测量。

[完整测量和验收记录](display-pipeline-acceptance.json)。

## 验证

- 在真实提交矩形上重建显示图像，逐像素核对滚屏、双向历史、彩色清屏、长行
  缩短、光标、边缘填充、失败重试。快照往返及截断报文、在途快照不受新输出
  修改、异步完成反馈保留pending输出均覆盖。
- runtime复制与libc比较双方32种偏移、双向重叠、0..8192字节及保护页边界。
  ASan/UBSan及泄漏检查覆盖fbterm与runtime，额外执行VM、文件和进程契约。
- QEMU单核/五核非阻塞I/O、poll、GPU、fbterm、键盘、滚轮、2000行完整输出、
  镜像关闭、嵌套PTY和退出恢复通过；GPU缺失路径保持。
- kernel函数文档5234项和228个非网络图身份同步；未运行fork性能门禁。
