#pragma once

#define SIGHUP 1
#define SIGINT 2
#define SIGKILL 9
#define SIGTERM 15
#define SIGCONT 18
#define SIGSTOP 19
#define SIGTSTP 20
#define SIGTTIN 21

// wait() still returns this project's plain exit status, not POSIX wait bits.
#define SIGNAL_EXIT_STATUS(signal) (128 + (signal))

#define SIGNAL_DEFAULT 0
#define SIGNAL_IGNORE 1
