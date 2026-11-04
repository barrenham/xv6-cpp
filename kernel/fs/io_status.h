#pragma once

// Internal kernel operations return negative errno; user I/O wrappers expose
// POSIX -1 with errno. Some older kernel paths still return generic -1.
#ifndef EAGAIN
#define EAGAIN 11
#endif
#ifndef EWOULDBLOCK
#define EWOULDBLOCK EAGAIN
#endif
