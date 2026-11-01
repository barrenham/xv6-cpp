# 像素复用、GPU完成等待与滚动历史

2026-11-01。本轮保留1920×1080及现有刷新节流，完成以下三项。

## 操作

启动QEMU并登录后运行 `fbterm`，或继续使用 `fbterm --no-serial-mirror`。

- PageUp / PageDown：在终端本地向上/下翻一页，重叠一行。
- VNC鼠标滚轮：每格移动3行；鼠标移动和点击不改变文本视口。
- End、输入普通字符或命令：返回实时画面；PageDown或向下滚动到末尾也可返回。
- 查看历史时隐藏光标，继续到达的输出仍处理，但不会把视口强行拉到底部。
- 最多保存1024行离屏历史；超过容量时淘汰最早的行。退出fbterm或清除历史
  的CSI 3J会丢弃历史，不写入持久文件。普通 `clear` 会发送CSI 3J。

串口和VNC PageUp/PageDown均支持。串口ESC[5~/ESC[6~跨read拆分时保留短前缀；
不匹配的转义序列原样送给子程序，孤立前缀最多等约30ms后原样转发。翻页键由
fbterm本地使用，未实现全屏程序/备用屏幕对这些键的独立接管。

## 像素复用

`Renderer`保存前一帧的字符、颜色和光标行快照，像素缓冲保持整屏绝对坐标。
通过累计滚屏位置减去历史偏移得到视口原点，确定旧像素行对应的新位置；
只有字符、颜色及光标都相同的行才可复用。上移按从上到下、下移按从下到上
复制，避免覆盖随后仍需要的源像素。新增或改变的行单独栅格化。

绘制前准备完整包围矩形，一次提交给GPU，保持已有合帧行为。提交失败会令
缓存失效，下次完整重绘；尺寸、终端重置/历史清除改变时也失效。光标所在旧
行不能直接当作无光标行复用，避免残影。底部/右侧非整字符边缘保持默认背景。

像素缓冲约7.91 MiB（1080p），行快照约31 KiB，1024×192×3字节历史存储
约0.56 MiB。历史单独在堆上分配，不把大零数组嵌入具有非零初值的终端对象，
避免ELF膨胀超出保留的xv6文件系统单文件上限；分配失败关闭设备并退出。

若一帧合并的输出跨过整个屏幕，旧行不能复用，仍需要完整栅格化。像素复用
减少用户态计算，**滚屏仍会提交整屏像素**，没有减少内核复制量或提供零拷贝。

## GPU完成中断

运行期提交后通过驱动平台桥接进入 `DeviceWaitFlow`，使用已有PollWait完成
通道和定时器：先订阅再重查used索引，完成则返回，否则睡眠等待IRQ或绝对
500ms期限。每次重试释放注册，期限不因唤醒而延长。IRQ确认设备状态后唤醒
等待者；没有SpinLock跨越睡眠，GPU原SleepLock仍串行保护一条在途请求。

启动时没有当前进程及可用IRQ路由，保留原来的有界轮询。完成后仍验证used
增量、描述符ID、响应长度、响应类型和fence。超时/信号/协议错误沿既有失败
路径重置并离线；重置未确认前继续保留DMA所有权。未增加自动重新初始化。

接口遵循VirtIO的used-buffer通知和evdev输入事件布局，见
[VirtIO 1.2规范](https://docs.oasis-open.org/virtio/virtio/v1.2/virtio-v1.2.html)。

root可通过XV_FB_GET_STATS读取固定32字节计数：interrupts、runtime_waits、
boot_polls、failed_waits。runtime_waits是进入并成功返回运行期等待路径的次数，
不等于调度器实际切换次数；完成抢先到达时不强行睡眠。

## 滚轮设备

QEMU图形/VNC目标新增 `virtio-mouse-device,bus=virtio-mmio-bus.5`；内核MMIO
第6槽0x10006000、IRQ6，映射6页。原有GPU/键盘槽位保持。`/dev/mouse0`
为30:1、root 0600，使用独立VirtIO输入实例、队列和独占打开，协商REL_WHEEL
能力；仍使用原8字节事件布局。只实现终端滚轮，没有图形指针或点击选择。
设备缺失时串口及键盘翻页仍可工作。

## 逐帧测量

客体内可使用：

```sh
env FBTERM_PROFILE=frames fbterm --benchmark-scroll
env FBTERM_PROFILE=frames FBTERM_RASTER=full fbterm --benchmark-scroll
```

固定60次单行滚屏，每次提交一帧，绕过PTY和日常节流，以隔离绘制工作。
首次画面/字体准备和报告输出不计入循环；每帧记录raster、draw、rows、reused，
原始时间单位为10MHz ticks，除以10000得到毫秒。一般交互会话的frames模式
只保留最近256个提交样本，停止后一次输出，避免实时日志影响刷新。

`FBTERM_RASTER=full`是同版本完整重绘参考路径；只有启用诊断时生效，默认
仍用像素缓存。两种模式都使用本轮的中断驱动，不能将此对照当成旧轮询驱动
的CPU使用率对比；也未给出单独中断节省CPU的百分比。

五核、1920×1080、无VNC客户端，每种模式两轮，共120帧，在其他回归开始前依次测量：

| 指标 | 完整重绘参考 | 像素复用 |
| --- | ---: | ---: |
| 每帧栅格化中位数 | 28.89 ms | 17.72 ms |
| 每帧栅格化P95 | 40.68 ms | 27.50 ms |
| 每帧重新栅格化行 | 54 | 2 |
| 每帧复用行 | 0 | 52 |

样本中栅格化中位耗时下降约39%。这是固定单行滚屏样本，不代表所有curl负载
都提升同样幅度。GPU DRAW中位仍约97～115ms、两种模式每60帧均提交
497664000字节；整屏复制/传输仍是主要限制，未承诺60fps。DRAW差值存在
调度波动，不能全部归因于像素复用。

每组两轮的GPU运行等待计数累计超过200，中断计数超过240，失败为0；各组
boot_polls在第二轮保持不变，表明运行期没有回到启动轮询路径。

宿主复现：

```sh
python3 tools/profile_fbterm.py --cpus 5 --rounds 2 --benchmark-scroll \
  --raster full --output build/docs/scrollback/full-frames.json
python3 tools/profile_fbterm.py --cpus 5 --rounds 2 --benchmark-scroll \
  --raster cached --output build/docs/scrollback/cached-frames.json
```

## 验证

- 宿主逐像素比较单行滚屏、双向视口移动、彩色编辑、光标、失败后重绘及边缘
  填充；单行仅重绘至多2行，其余复用。查看历史时新输出不产生重复GPU提交。
- 1200行输入核对1024行环形历史淘汰、边界夹取、返回实时及CSI 3J清理。
- 串口导航前缀跨输入调用、30ms超时、非翻页转义原样透传。
- 模拟GPU延迟完成：transfer和flush各经完成IRQ唤醒，仍通过fence和响应校验；
  原有超时、错误响应和DMA回收契约保持。
- 单核/五核实际QMP画面核对串口翻页、VNC PageUp/PageDown、双向滚轮及回到
  实时后继续命令；保留2000行完整输出、嵌套PTY逐层退出及镜像关闭检查。
- 非阻塞I/O、poll、GPU存在/缺失及fbterm无设备路径回归；ASan/UBSan/泄漏检查。
- 串口长命令仍核对全部字节；发送改为按真实回显推进，避免固定宿主睡眠间隔
  在同步绘制期间溢出原有128字节输入环。未改变输入容量，也未缩短长命令。
- kernel函数文档5234项、228个非网络图身份；新增DeviceWaitFlow，既有条件图
  和网络子树保持。fork门禁按用户要求跳过。

[验收记录与逐帧数据](scrollback-acceptance.json)。重启QEMU使用新内核及fbterm。

后续输出/显示解耦、裁剪提交与复制优化见[显示管道优化](display-pipeline.md)。
