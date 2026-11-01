# 共享 framebuffer 映射与 GPU 视口滚屏

2026-11-01。fbterm默认直接写入GPU像素页，不再为每帧执行用户像素缓冲到
内核framebuffer的复制。GPU资源扩展为两屏高，滚屏时通过移动scanout窗口
复用已有资源，只上传新出现或变化的行。保持可见分辨率1920×1080，并按增量或整屏重绘调整合帧预算。

## 从用户指针到同一份物理像素页

`XV_FB_MAP`把GPU驱动分配的散页，以连续的用户虚拟地址映射进当前进程。
用户态和设备访问同一份物理像素存储；不是先写普通malloc内存再在ioctl里复制。
此处提供设备专用的mmap式接口，没有扩展为通用文件mmap系统调用。

```text
fbterm 用户虚拟地址 ── 用户页表 ──┐
                                ├── GPU backing 物理页
内核直映地址 ────────────────────┘          │
                                    TRANSFER_TO_HOST_2D
                                           │
                                    QEMU 显示资源 → VNC
```

共享窗口放在UTOP下方32MiB处，最多4096页；不计入普通堆大小，区域登记防止
sbrk增长覆盖它。权限为用户可读写、不可执行。像素页之外的命令队列、响应页和
MMIO寄存器不会映射给用户。1080p资源为1920×2160×4字节，约15.82MiB。

每个地址空间对每个映射页保留引用，驱动也持有自己的引用：

- fork保留同一组共享可写页，设备窗口不经过普通堆的COW转换。
- exec替换映像、地址空间reset/回收及显式UNMAP释放映射引用。
- swap交接映射计数和身份；解除后经用户态返回路径刷新TLB。
- MAP失败会回滚已完成的页前缀；向用户发布描述失败只撤销新映射，保留旧映射。
- 设备失败仍先reset，确认停止DMA后释放驱动引用；仍被用户映射的页不会回收。
  复位未确认时沿原路径保留DMA所有权。

关闭fd不会自动撤销映射。首次映射及PRESENT/统计操作要求root；已建立映射
相当于已授予的内存访问能力，降权不会自动撤销它，exec会清除旧映射。

## GPU复用旧行，而非CPU搬像素

Renderer保留上一帧的字符、颜色和光标快照。累计滚屏位置的差值决定新窗口
原点：旧的第N行仍位于相同的资源物理位置，只是它在显示窗口里的行号改变。
字符、颜色及光标相同的行既不搬运像素，也不重新上传；变化行才重新栅格化。

一次呈现按以下顺序执行，每条命令都等待带fence的完成响应：

1. 对脏区执行`TRANSFER_TO_HOST_2D`，没有脏区时跳过。
2. 原点改变时执行`SET_SCANOUT`，选择同一资源中的另一个可见矩形。
3. 执行`RESOURCE_FLUSH`；窗口改变时通知整个可见窗口，普通编辑刷新脏区。

两屏高空间不能容纳无限滚屏。原点将越界时回到0并完整重绘可见窗口，之后继续
滑动。第一次绘制初始化整个资源，防止尚未使用的右侧/离屏区域显示黑底。
资源保留曾写入内容的最大宽度，脏行按这个保守边界上传，擦除更早的离屏长行尾部；
初始资源清屏外不必总是上传整宽。旧行仍保留在资源中。
历史双向翻页、清屏、光标移动、部分字符高度的底部边缘和提交失败重绘均支持。

这是VirtIO基础2D资源与scanout操作，没有引入VirGL、3D命令、垂直同步或硬件
原子换页。命令顺序遵循[GPU设备规范](https://github.com/oasis-tcs/virtio-spec/blob/master/device-types/gpu/description.tex)
的资源上传、显示及fence完成语义。QEMU/VNC仍需要处理可见像素变化，上传量
减少不等于VNC网络流量按同样比例减少。

## 接口

原`GET_INFO`、`DRAW`及两个计时/完成统计接口保持原布局，旧绘图应用仍工作；
旧DRAW在需要时把scanout恢复到资源原点0。GPU专用队列从16槽扩为32槽，
以容纳两屏高资源最多16个请求页加响应页；其他VirtIO设备队列不变。

| 接口 | 描述 |
| --- | --- |
| `XV_FB_MAP` | 输出24字节`FramebufferMapping`：address、bytes、backing_height、version=1 |
| `XV_FB_UNMAP` | 解除调用进程的设备窗口；设备已离线也可解除；不存在则返回-1 |
| `XV_FB_PRESENT` | 输入24字节矩形和origin_y；像素已在共享页中，禁止越界及非零reserved |
| `XV_FB_GET_MAPPED_STATS` | 32字节：首次成功MAP次数、成功PRESENT次数、上传字节量、scanout变化次数 |

PRESENT的x/y是相对于请求窗口原点的资源坐标；允许向窗口下方的离屏资源上传，
但不超过backing高度。width/height可以同时为0，只调整窗口。映射身份和页数
由内核保留并核对，用户不能指定任意物理地址。元数据仍需要小量copy_in/out；
“零拷贝”在此指用户到内核的像素复制，不包括QEMU内部的资源传输与VNC编码。

## 运行与诊断

更新根盘中的fbterm/fbdemo并重启QEMU后，普通使用方式不变：

```sh
fbterm --no-serial-mirror
```

已有独立显示进程继续负责绘图，主进程处理PTY与输入。持续批量输出且整屏重绘时，下一批至少合并250ms；
增量滚屏保留100ms下限，普通输入保留短延迟。主进程提交快照后清除本批的
scrolled标记，后续字符不会永久继承批量输出状态。映射由fork正确继承，
不用通过管道传送像素，也没有每个进程各自的malloc像素数组；仍只发送字符快照。
常驻GPU资源比旧版多一屏，用户像素堆及其COW副本则不再需要。同一资源的
多个映射共享像素，应用需协调并发绘制；本轮未加入显示服务或多窗口合成。

仅诊断模式支持对照：

```sh
env FBTERM_PROFILE=frames FBTERM_BUFFER=copied FBTERM_SCROLL=copied fbterm --benchmark-scroll
env FBTERM_PROFILE=frames FBTERM_SCROLL=copied fbterm --benchmark-scroll
env FBTERM_PROFILE=frames fbterm --benchmark-scroll
```

分别为原复制提交、共享映射但CPU搬行、共享映射加GPU窗口复用。FBSTAGES的
copied/copy_ms在正常映射路径应为0；FBMAPPED记录呈现、实际资源上传和窗口切换。
计时为墙钟时间，包含调度和设备等待，不等同于CPU忙碌时间。SET_SCANOUT
开销包含在draw中，未单独分段计时。固定60帧诊断绕过合帧节流，不能推算
实际交互刷新率。

## 本次测量

五核、1920×1080，每种路径两轮，每轮固定60次单行滚屏；同一内核、同一
渲染器，分别选择复制缓冲、共享缓冲和视口复用。初始化、预热和报告在循环
外；各客体顺序运行，没有并行回归客体。

| 指标 | 复制缓冲、CPU搬行 | 共享缓冲、CPU搬行 | 共享缓冲、GPU视口 |
| --- | ---: | ---: | ---: |
| 60帧循环耗时中位数 | 2.691 s | 1.718 s | 0.406 s |
| DRAW单帧中位数 | 19.22 ms | 6.50 ms | 4.34 ms |
| 栅格化/状态处理单帧中位数 | 24.69 ms | 20.98 ms | 1.55 ms |
| 内核像素复制平均每帧 | 11.85 ms | 0 | 0 |
| 用户旧像素搬运平均每帧 | 14.93 ms | 12.95 ms | 0 |
| 每轮用户→内核像素复制 | 127.01 MB | 0 | 0 |
| 每轮GPU资源上传 | 127.01 MB | 127.01 MB | 6.74 MB |
| 整条命令QEMU CPU秒中位数 | 10.11 s | 9.09 s | 7.50 s |

视口组每轮发生60次scanout变化，包含一次窗口回卷及完整可见窗口重绘。
资源上传减少约94.7%，固定循环耗时下降约84.9%。CPU秒的范围比循环更广，
包括不同路径的映射、初始资源绘制和报告，不能当作单线程CPU忙碌时间。
两轮样本不足以承诺所有输出都有相同提升。另做一轮活跃Raw VNC，60帧循环
0.482s，复制仍为0、上传6.74 MB；不代表远程网络与MobaXterm的同等延迟。

2000行PTY输出（完整88013字节，关闭串口镜像）另行对照：

| 指标 | 复制缓冲 | 共享缓冲及视口 |
| --- | ---: | ---: |
| transfer循环耗时中位数 | 1.617 s | 1.784 s |
| 每轮提交帧数 | 4 / 4 | 5 / 6 |
| 每轮资源上传 | 7.26 / 7.26 MB | 9.07 / 10.77 MB |
| 整条命令QEMU CPU秒中位数 | 4.92 s | 4.58 s |

此批量负载总耗时没有改善；一次跳过很多行时缺少可复用的可见行，而更快的
显示路径也改变了主循环/工作进程调度及中间帧数量。文本解析、字符网格滚动、
PTY和调度仍有成本。CPU秒差值较小且仅两轮，不能断言普遍降低CPU使用率。
以上反例保留在[完整验收和原始数据](mapped-gpu-acceptance.json)，没有用固定
单行滚屏结果替代大批量输出的吞吐测量。

宿主复现：

```sh
python3 tools/profile_fbterm.py --cpus 5 --rounds 2 --benchmark-scroll \
  --buffer copied --scroll copied --output build/docs/mapped-gpu/copied.json
python3 tools/profile_fbterm.py --cpus 5 --rounds 2 --benchmark-scroll \
  --scroll copied --output build/docs/mapped-gpu/mapped-copy-scroll.json
python3 tools/profile_fbterm.py --cpus 5 --rounds 2 --benchmark-scroll \
  --output build/docs/mapped-gpu/mapped-viewport.json
```

## 验证范围

- 真实QEMU单核/五核运行`fbdemo --mapped`，验证fork共享写入、exec清理、
  描述发布失败回滚、显式解除、越界拒绝及像素复制计数不增加；从第二屏选择
  画面，并通过QMP和真实TCP VNC逐像素核对RGB色条。
- VM契约覆盖buddy/freelist两种分配器、共享写入不触发COW、不可执行、映射
  身份、swap交接、reset/解除后的引用及页表跨2MiB边界时的OOM前缀回滚。
- GPU模拟设备保留全部DMA分配失败点、超时和坏fence检查，增加映射描述失败、
  脏区上传/窗口选择/flush顺序、零像素复制及offline后最后引用释放。
- Renderer使用独立“宿主资源”模型，只复制实际提交区域，再逐像素对照终端。
  覆盖滚屏、窗口回卷及离屏长行尾部回收、彩色背景、宽行擦除、双向历史、光标、底部
  非整字符边缘及提交失败重绘；另执行ASan/UBSan与泄漏检查。
- 保留串口镜像、完整2000行输出、嵌套PTY、键盘/滚轮、EOF/退出恢复回归。
- kernel函数文档5260项、231个非网络图身份同步。未执行fork性能门禁。
