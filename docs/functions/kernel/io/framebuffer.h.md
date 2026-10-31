# `kernel/io/framebuffer.h`

共享ABI布局与常量；行为由调用模块实现。

XV_FB_GET_STATS返回固定32字节FramebufferStats，只读root诊断计数，
包含IRQ、运行等待、启动轮询及失败等待；原有GET_INFO/DRAW布局不变。
