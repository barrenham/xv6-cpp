# 内核打印服务

`import xv.print;` 提供 `xv::print::initialize()`、`xv::print::printk()`
和 `xv::print::panic()`。引导核先初始化 console，再初始化打印锁，之后发布
启动完成状态供其他 hart 使用。

`print.cppm` 定义公开模块接口，`printk.cpp` 是模块实现单元，负责格式化、
打印锁和 panic 输出。现有子系统及原生测试继续使用 `print_abi.h` 中的
`printk/panic` C ABI；模块导出同一组函数，不维护第二份打印实现。
`defs.h` 包含该兼容头，不再声明打印或内核页表初始化函数。
