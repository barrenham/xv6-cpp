# `kernel/process/exec/elf_loader.cppm`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../kernel/process/exec/elf_loader.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process:elf_loader`。

直接模块依赖：`:executable`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::process::ElfLoader::ElfLoader`](elf_loader.cppm.md#fn-ff415c9c2e69f1e8)
- [`xv::process::ElfLoader::load`](elf_loader.cpp.md#fn-7873c234f30790ee)
- [`xv::process::ElfLoader::name`](elf_loader.cppm.md#fn-afe065db62eee462)

<a id="fn-ff415c9c2e69f1e8"></a>

## `xv::process::ElfLoader::ElfLoader`

```doxygen
@fn constexpr xv::process::ElfLoader::ElfLoader()
@brief 构造ELF 映像加载器，建立本重载要求的成员初值和借用关系。
@details 使用编译器生成的默认成员行为。普通标量是否被初始化取决于成员默认初始化器和调用方的值初始化方式，不能把 default 一概当作清零。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-afe065db62eee462"></a>

## `xv::process::ElfLoader::name`

```doxygen
@fn const char *xv::process::ElfLoader::name() const override
@brief ELF 映像加载器：读取或计算 `"`，返回当前查询结果。
@return 各分支实际返回：无值返回。应结合上面的条件检查区分正常值、空值和错误值。
```
