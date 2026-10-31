# `kernel/fs/buffer/snapshot_text.cppm`

<a id="fn-25cc5f76459a505b"></a>

## `xv::fs::SnapshotText::SnapshotText`

```doxygen
@fn explicit xv::fs::SnapshotText::SnapshotText(PageBuffer &buffer):buffer_(buffer)
@brief 构造对象并建立成员初值及借用关系。
@details 基于PageBuffer的快照文本格式化；写入错误保持在failed_中，由good()统一报告。
```

<a id="fn-4f7c2f62c8ca2dcb"></a>

## `xv::fs::SnapshotText::raw`

```doxygen
@fn void xv::fs::SnapshotText::raw(const void *data,unsigned count)
@brief 追加原始字节，失败设置粘性错误。
@details 基于PageBuffer的快照文本格式化；写入错误保持在failed_中，由good()统一报告。
```

<a id="fn-f34437c5df164541"></a>

## `xv::fs::SnapshotText::append`

```doxygen
@fn void xv::fs::SnapshotText::append(const char *text)
@brief 追加NUL终止文本，终止符不写入快照。
@details 基于PageBuffer的快照文本格式化；写入错误保持在failed_中，由good()统一报告。
```

<a id="fn-71810e2ce6eee1d4"></a>

## `xv::fs::SnapshotText::number`

```doxygen
@fn void xv::fs::SnapshotText::number(unsigned long value,unsigned radix=10)
@brief 以指定进制输出一个无符号整数。
@details 基于PageBuffer的快照文本格式化；写入错误保持在failed_中，由good()统一报告。
```

<a id="fn-207dc45a01b61334"></a>

## `xv::fs::SnapshotText::field`

```doxygen
@fn void xv::fs::SnapshotText::field(const char *name,unsigned long value,const char *unit="\n")
@brief 输出字段名、整数和单位。
@details 基于PageBuffer的快照文本格式化；写入错误保持在failed_中，由good()统一报告。
```

<a id="fn-3ee54eaafc06d36a"></a>

## `xv::fs::SnapshotText::hexadecimal`

```doxygen
@fn void xv::fs::SnapshotText::hexadecimal(const char *name,unsigned long value)
@brief 输出带0x前缀的十六进制字段。
@details 基于PageBuffer的快照文本格式化；写入错误保持在failed_中，由good()统一报告。
```

<a id="fn-916b8502d81a0d8d"></a>

## `xv::fs::SnapshotText::good`

```doxygen
@fn bool xv::fs::SnapshotText::good() const
@brief 查询快照格式化过程中是否曾发生写入失败。
@details 基于PageBuffer的快照文本格式化；写入错误保持在failed_中，由good()统一报告。
```
