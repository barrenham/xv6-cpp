# `kernel/drivers/rtc/goldfish.cppm`

<a id="fn-5ea474bfe4c038cb"></a>

## `xv::drivers::GoldfishRtc::GoldfishRtc`

```doxygen
@fn constexpr explicit xv::drivers::GoldfishRtc::GoldfishRtc(unsigned long base):registers_(base)
@brief 构造对象并建立成员初值及借用关系。
@details Goldfish RTC访问：low读取锁存high，自旋锁与I/O barrier固定这对读取，防止多核交叉读取。
```

<a id="fn-5a0a9ced65c11e52"></a>

## `xv::drivers::GoldfishRtc::name`

```doxygen
@fn const char *xv::drivers::GoldfishRtc::name() const override
@brief 返回 `"Goldfish RTC"`。
@details Goldfish RTC访问：low读取锁存high，自旋锁与I/O barrier固定这对读取，防止多核交叉读取。
```

<a id="fn-520f1a7fe7ceb9d8"></a>

## `xv::drivers::GoldfishRtc::initialize`

```doxygen
@fn void xv::drivers::GoldfishRtc::initialize() override
@brief 实现 xv::drivers::GoldfishRtc::initialize 的模块操作，按声明参数读写其封装状态。
@details Goldfish RTC访问：low读取锁存high，自旋锁与I/O barrier固定这对读取，防止多核交叉读取。
```

<a id="fn-a6a5527a68bffe8f"></a>

## `xv::drivers::GoldfishRtc::nanoseconds`

```doxygen
@fn unsigned long xv::drivers::GoldfishRtc::nanoseconds()
@brief 返回 `(static_cast<unsigned long>(high)<<32)|low`。
@details Goldfish RTC访问：low读取锁存high，自旋锁与I/O barrier固定这对读取，防止多核交叉读取。
```

<a id="fn-f5eaf86219e89b93"></a>

## `xv::drivers::GoldfishRtc::seconds`

```doxygen
@fn unsigned long xv::drivers::GoldfishRtc::seconds()
@brief 返回 `nanoseconds()/1000000000UL`。
@details Goldfish RTC访问：low读取锁存high，自旋锁与I/O barrier固定这对读取，防止多核交叉读取。
```
