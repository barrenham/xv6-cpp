# 用户应用

| 分类 | 内容 |
| --- | --- |
| accounts | 登录、身份、账户与用户组 |
| coreutils | 文本流、环境与基本命令 |
| examples | 示例程序，安装到 /usr/tests |
| filesystem | 文件操作、挂载、FS 控制与后端示例 |
| network | 网络配置、过滤规则、DNS、HTTP 与 ping |
| shell | Shell 语法、执行、作业、编辑与环境 |
| system | 启动、服务监督、驱动控制和系统信息 |
| terminal | 图形终端、PTY、终端设置和显示示例 |

每个应用的 main.cpp 是入口，application.h 声明应用入口契约，实际行为位于
独立实现文件；复杂应用再按职责拆分。sources.cmake 是应用自己的源文件清单，
apps/programs.cmake 负责注册应用，user/CMakeLists.txt 统一链接运行库和生成安装项。

测试留在 user/tests，可复用库分别位于 user/runtime、user/fs 和 user/services。
不要在应用之间包含另一个应用的 .cpp；共享能力放入对应库。
