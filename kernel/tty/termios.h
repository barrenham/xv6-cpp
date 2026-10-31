#pragma once

// Shared kernel/userspace ABI. Layout and numbers follow Linux asm-generic
// TCGETS (not the libc termios2/speed extension). Unsupported flags are rejected.
typedef unsigned int tcflag_t;
typedef unsigned char cc_t;
#define NCCS 19

struct termios
{
  tcflag_t c_iflag;
  tcflag_t c_oflag;
  tcflag_t c_cflag;
  tcflag_t c_lflag;
  cc_t c_line;
  cc_t c_cc[NCCS];
};

struct winsize
{
  unsigned short ws_row, ws_col, ws_xpixel, ws_ypixel;
};

#define VINTR 0
#define VQUIT 1
#define VERASE 2
#define VKILL 3
#define VEOF 4
#define VTIME 5
#define VMIN 6
#define VSTART 8
#define VSTOP 9
#define VSUSP 10
#define VEOL 11
#define VWERASE 14
#define VLNEXT 15
#define VEOL2 16
#define IGNCR 0x0080
#define ICRNL 0x0100
#define INLCR 0x0040
#define ISTRIP 0x0020
#define IXON 0x0400
#define OPOST 0x0001
#define ONLCR 0x0004
#define CSIZE 0x0030
#define CS8 0x0030
#define CREAD 0x0080
#define PARENB 0x0100
#define CLOCAL 0x0800
#define B38400 0x000f
#define ISIG 0x0001
#define ICANON 0x0002
#define ECHO 0x0008
#define ECHOE 0x0010
#define ECHOK 0x0020
#define ECHONL 0x0040
#define ECHOCTL 0x0200
#define ECHOKE 0x0800
#define IEXTEN 0x8000
// xv6 advisory flag: this terminal is being forwarded by a foreground proxy.
// Saved/restored with termios; outer proxies pass their detach key inward.
#define XV_PROXY 0x80000000U
#define TCGETS 0x5401
#define TCSETS 0x5402
#define TCSETSW 0x5403
#define TCSETSF 0x5404
#define TCFLSH 0x540b
#define TIOCSCTTY 0x540e
#define TIOCGPGRP 0x540f
#define TIOCSPGRP 0x5410
#define TIOCGWINSZ 0x5413
#define TIOCSWINSZ 0x5414
#define FIONREAD 0x541b
#define TIOCGPTN 0x80045430
#define TIOCSPTLCK 0x40045431
#define TCSANOW 0
#define TCSADRAIN 1
#define TCSAFLUSH 2
#define TCIFLUSH 0
// Output flush requests are currently unsupported by both UART and PTY.
#define TCOFLUSH 1
#define TCIOFLUSH 2
