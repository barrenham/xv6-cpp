# kernel 独立 Doxygen 函数文档

已按 [kernel 全量导航](kernel/README.md) 覆盖 249 个源码/接口/共享 include/汇编文件，
登记 1488 个函数或控制流入口条目。实际代码仍在 `kernel/`，文档集中在 `docs/`；
不向源码添加说明、不生成额外 C++ 声明、不使用编辑器插件或 LSP 代理。

## 范围与阅读方式

- 默认 dev 配置由真实 clangd 清点出 2200 处声明/定义，按 USR 归并为 1423 个唯一函数项。
- 包含普通/静态函数、成员函数、内联与模板、构造/析构、运算符、删除成员及纯虚契约。
- 额外记录 49 个共享寄存器函数、13 个仅条件配置接口，以及未作为 C++ 函数声明的 3 个汇编入口。
  swtch/kernelvec 的已有 C++ 声明与汇编实现共用条目，不重复计数。
- 宏、常量、类型、布局及仅重导出的模块，在对应文件概览中说明，不计为可调用函数。

每个文件页都列出其声明或定义对应的唯一条目，避免在接口和实现中维护两套函数说明。
原有等待/睡眠/生命周期等 30 篇详细条目保留；其余说明结合函数实际结构记录参数、
调用/构造表达式、状态写入、分支与返回值，关键路径再补充锁、资源和调用协议。
**全量条目覆盖不等于逐行人工审查完成，也不等于每个位置都能覆盖 clangd 的原有悬浮。**

## 工具与产物

| 路径 | 职责 |
| --- | --- |
| `docs/functions/kernel/` | 独立 Doxygen Markdown 与逐文件导航 |
| `docs/functions/catalog.json` | USR、签名、函数体指纹和文档锚点绑定 |
| `docs/functions/files.json` | kernel 源文件覆盖清单 |
| `tools/clangd_docs/inventory.py` | 从真实 clangd 清点声明、定义及扩展配置 |
| `tools/clangd_docs/coverage.py` | 检查遗漏、失效绑定、源码变化和链接 |
| `tools/clangd_docs/build.py` | 生成原生 YAML 符号索引 |
| `tools/clangd_docs/verify.py` | 验证真实 hover、符号身份和跳转 |
| `build/docs/` | 被 Git 忽略的索引、清点缓存、覆盖报告和实际 hover 样本 |

## 更新与验证

在项目根目录运行。先确保 dev 已构建且 BMIs 与源码一致；纯文档变化不需要重跑 QEMU。

```sh
export CONDA_PREFIX=/home/barrenham/miniconda3/envs/OS-LAB
export PATH="$CONDA_PREFIX/bin:$PATH"
python tools/clangd_docs/inventory.py --clangd "$CONDA_PREFIX/bin/clangd"
python tools/clangd_docs/coverage.py --clangd "$CONDA_PREFIX/bin/clangd"
python tools/clangd_docs/build.py --clangd "$CONDA_PREFIX/bin/clangd"
python tools/clangd_docs/verify.py --clangd "$CONDA_PREFIX/bin/clangd"
```

需要检查与编辑器相同的后台索引组合时，给 verify 加 `--background-index`。
索引生成后，在 VS Code 执行 **clangd: Restart language server**；其他标准 LSP
编辑器重启其 clangd 即可。修改同名索引文件不保证被运行中的 clangd 热加载。

清点缓存的身份包含 kernel 所有相关文件指纹、编译数据库和 clangd 版本；失配时
必须重新清点。绑定使用真实 USR/ID 和 AST 选择范围，处理重载、构造/析构和局部方法，
不按行号或同名字符串猜测。函数体或签名变化后，coverage 会要求重新核对文档并更新
`source_sha256`；不要机械刷新指纹来绕过行为说明的维护。

旧的 30 项兼容入口仍保留签名定位；新条目统一使用编译器语义绑定。索引全部解析成功
后才原子替换。复杂声明和原生 hover 是否可用，仍以实际验证报告为准。

## Doxygen 格式

每个函数一个稳定锚点和一个 `doxygen` 代码块，使用 @fn、@brief、@param、@return、
@details、@pre、@note、@see。@fn 在写入 Documentation 前剥除，其余交给 clangd 渲染。
当前版本会组织 Brief/Parameters/Returns/Details/Note；@pre/@see 等可能以标签文字显示。
删除成员明确说明编译期禁止，纯虚成员说明派生契约，不编造未实现的函数体。

`.clangd` 通过 `Index.External.File` 加载 `build/docs/functions.idx`，并使用
`Documentation.CommentFormat: Doxygen`。外部索引片段后显式恢复 `Index.Background: Build`，
不让文档索引取代完整项目语义索引。实测工具链为 clangd 23.1.4。

## 原生悬浮的边界

全量验证逐项区分：外部 Doxygen 实际显示、源码文档优先/原生索引限制、仅独立文档。
不能把“写进索引”当成“已经显示”。源码已有短注释时 clangd 会优先使用它；普通 `//`
也可能被收集，传入 `-fno-parse-all-comments` 不能覆盖其强制解析行为。

共享 `.inc` 在用户兼容头与 `xv.arch` 中具有不同声明上下文，条件编译条目不属于默认
配置，汇编标签也不是普通 C++ 函数。这些项目保留独立文档，但不向默认索引塞入猜测 ID。
默认 LAB_NET 已构建并通过网络测试；额外 LAB_LOCK 的扩展清点只是解析边界；例如 stats.cpp 仍有诊断，不能宣称完成该实验
配置的内核构建。具体条目与原因记录在 `build/docs/hover-report.json`。

官方机制依据：[悬浮优先级](https://github.com/llvm/llvm-project/blob/main/clang-tools-extra/clangd/Hover.cpp)、
[普通注释解析](https://github.com/llvm/llvm-project/blob/main/clang-tools-extra/clangd/Compiler.cpp)、
[索引 Documentation 字段](https://github.com/llvm/llvm-project/blob/main/clang-tools-extra/clangd/index/YAMLSerialization.cpp)、
[配置](https://clangd.llvm.org/config)。

本轮接口抽象后的实际验证：1253 项显示外部 Doxygen，10 项源码注释优先，160 项受原生注释优先级或索引限制，65 项仅保留独立文档；另有 5 个跨模块 Doxygen 探针通过。完整结果见 `build/docs/hover-report.json`，代码变化后需重新验证。
