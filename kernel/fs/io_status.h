#pragma once

// This kernel returns -EAGAIN directly; legacy errors remain -1, no errno TLS.
#ifndef EAGAIN
#define EAGAIN 11
#endif
#ifndef EWOULDBLOCK
#define EWOULDBLOCK EAGAIN
#endif
