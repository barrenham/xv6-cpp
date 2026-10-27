# `kernel/process/credentials/credentials.cppm`

## 函数导航

- [`xv::process::Credentials::Credentials`](credentials.cppm.md#fn-f8c2bdf5a54d0b00)
- [`xv::process::Credentials::inherit`](credentials.cppm.md#fn-e461a6eea2969afc)
- [`xv::process::Credentials::may_signal`](credentials.cppm.md#fn-66a68733c0d3750d)
- [`xv::process::Credentials::set_gid`](credentials.cppm.md#fn-6de91f06a7cca1b8)
- [`xv::process::Credentials::set_groups`](credentials.cppm.md#fn-e407d355ea70c0d4)
- [`xv::process::Credentials::set_mask`](credentials.cppm.md#fn-7c8ea4484564c0a5)
- [`xv::process::Credentials::set_uid`](credentials.cppm.md#fn-7129ca19467a2a79)
- [`xv::process::Credentials::snapshot`](credentials.cppm.md#fn-91605e3a2b55b5ba)

<a id="fn-f8c2bdf5a54d0b00"></a>

## `xv::process::Credentials::Credentials`

```doxygen
@fn constexpr xv::process::Credentials::Credentials() = default
@brief 建立默认 root 进程凭据。
@details 默认真实、有效、保存 UID/GID 为 0，附加组为空，umask 为 0022；锁和值直接初始化，不分配资源。
```

<a id="fn-91605e3a2b55b5ba"></a>

## `xv::process::Credentials::snapshot`

```doxygen
@fn CredentialState xv::process::Credentials::snapshot() const
@brief 取得一致的凭据值快照。
@details 短暂持独立自旋锁复制整个 CredentialState，包含真实/有效/保存身份、附加组及 umask；返回值可在释放锁后检查。
```

<a id="fn-e461a6eea2969afc"></a>

## `xv::process::Credentials::inherit`

```doxygen
@fn void xv::process::Credentials::inherit(const Credentials &parent)
@brief 继承父进程凭据。
@details 先复制父快照再锁定子对象写入，避免同时持两把凭据锁。用于尚未发布的 fork 子进程，之后父子修改互不影响。
```

<a id="fn-7129ca19467a2a79"></a>

## `xv::process::Credentials::set_uid`

```doxygen
@fn int xv::process::Credentials::set_uid(unsigned value, bool effective_only = false)
@brief 按权限修改真实、有效及保存 UID。
@details 仅接受 0..65534。非 root 仅可把有效 UID 切换到真实或保存 UID；root 的 setuid 同时修改三者，seteuid 只改有效值。验证与修改在同一锁内完成，失败返回 -1。
```

<a id="fn-6de91f06a7cca1b8"></a>

## `xv::process::Credentials::set_gid`

```doxygen
@fn int xv::process::Credentials::set_gid(unsigned value, bool effective_only = false)
@brief 按权限修改真实、有效及保存 GID。
@details 范围 0..65534；权限由有效 UID 是否为 root 决定。root 的 setgid 改三者，setegid 只改有效组；非 root 只允许真实或保存组。不会自动修改附加组。
```

<a id="fn-e407d355ea70c0d4"></a>

## `xv::process::Credentials::set_groups`

```doxygen
@fn int xv::process::Credentials::set_groups(const unsigned *groups, unsigned count)
@brief 替换附加组列表。
@details 先验证最多 16 项和 ID 范围，再持凭据锁确认有效 UID 为 0 并整体替换；失败保持原列表，count=0 清空。
```

<a id="fn-7c8ea4484564c0a5"></a>

## `xv::process::Credentials::set_mask`

```doxygen
@fn unsigned xv::process::Credentials::set_mask(unsigned mask)
@brief 设置创建掩码并返回旧值。
@details 持凭据锁仅保留 0777；fork 复制，exec 保留。它影响后续新节点创建，不修改已有文件。
```

<a id="fn-66a68733c0d3750d"></a>

## `xv::process::Credentials::may_signal`

```doxygen
@fn static bool xv::process::Credentials::may_signal(const CredentialState &sender, const CredentialState &target)
@brief 判断用户进程是否可发送信号。
@details root 放行；否则发送者真实或有效 UID 必须匹配目标真实或保存 UID。纯值判断不加锁，调用方提供一致快照；内核终端通知不使用此用户授权路径。
```
