# `kernel/io/keyboard.h`

8字节键盘事件ABI及同步丢失常量，无函数条目；不是Linux input_event二进制布局。

新增EV_REL/REL_WHEEL常量。mouse0与kbd0使用独立设备实例和队列，
共享原8字节事件布局；滚轮value按32位有符号增量解释。
