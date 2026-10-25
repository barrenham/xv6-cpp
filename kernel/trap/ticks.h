#pragma once

// Include after importing xv.sync. The timer lock is shared with sys_pause().
extern "C" xv::sync::SpinLock tickslock;
