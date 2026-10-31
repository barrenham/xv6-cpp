# 显示分辨率

默认图形分辨率为 **1280×800**。`fbterm` 根据 `/dev/fb0` 返回的尺寸，
使用10×20字符单元自动建立 **128列、40行** 的网格，并同步设置PTY窗口和
COLUMNS/LINES环境变量。VNC键盘与串口输入保持可用。

## 配置

```sh
conda activate OS-LAB
cmake --preset dev -DXV_DISPLAY_WIDTH=1280 -DXV_DISPLAY_HEIGHT=800
cmake --build --preset dev --target qemu-vnc
```

也可以使用1920×1080：

```sh
cmake --preset dev -DXV_DISPLAY_WIDTH=1920 -DXV_DISPLAY_HEIGHT=1080
cmake --build --preset dev --target qemu-vnc
```

允许宽度640..1920、高度480..1080，均为正整数；越界或非整数配置会报错。
分辨率在构建和启动时确定，GPU资源、QEMU xres/yres及VNC画面一致。
修改后需要退出旧QEMU并重新启动，不提供运行期模式切换。已有根盘无需重建。
升级到本版时先更新 `fbterm` 和 `fbdemo`；它们读取实际尺寸，之后调整分辨率
只需要重新构建内核并启动。离线更新步骤见 [rootfs工具](rootfs-tools.md)。

| 画面尺寸 | fbterm列×行 | 未用于完整字符的边缘 |
| --- | --- | --- |
| 640×480 | 64×24 | 无 |
| 1280×800（默认） | 128×40 | 无 |
| 1366×768 | 136×38 | 右侧6像素、底部8像素 |
| 1920×1080 | 192×54 | 无 |

不足一个字符单元的右侧或底部使用背景色，不拉伸字形。字体使用灰度等宽ASCII字形，边缘进行覆盖率混合；
不包含中文字体。字形与滚屏修复见 [终端渲染](terminal-rendering.md)。

## 驱动与内存

GPU独立采用16槽描述符/available/used环；磁盘和熵设备的原8槽环不变。
显存仍由散页组成，`FramePages=ceil(width*height*4/4096)`；attach请求页数为
`ceil((32+FramePages*16)/4096)`，控制页数为请求页数加4。响应页紧随所有
请求页，避免大分辨率的backing条目覆盖响应缓冲。最大请求需要8页加一个
响应描述符，处于16槽界限内。

| 分辨率 | 帧缓冲页 | 控制页 | GPU总页数 |
| --- | ---: | ---: | ---: |
| 640×480 | 300 | 6 | 306 |
| 1280×800 | 1000 | 8 | 1008 |
| 1920×1080 | 2025 | 12 | 2037 |

无GPU时仍不分配这些DMA页。FENCE、响应类型/长度/索引校验、root权限、
500ms命令期限和reset确认后释放DMA的约束保留。

fbterm网格容量支持最大192×54单元，实际行列由GET_INFO决定。绘制合并全部
脏行为一次矩形提交，像素缓冲按实际屏幕分配。`fbdemo` 的行缓冲支持最大
1920像素。实现与内存说明见 [终端渲染](terminal-rendering.md)。

## 验证范围

1280×800运行GPU、fbterm、VNC键盘和缺失设备检查；1920×1080运行真实
VNC全部像素核对及键盘交互；640×480验证保留的低分辨率。宿主契约按当前
尺寸覆盖全部DMA分配失败点、跨多页attach、右下角绘制及越界拒绝。
终端契约覆盖动态网格、自动折行、滚动、最大网格和非整字符边缘。

截图、日志和验收范围见 [分辨率验收](display-resolution-acceptance.json)。
