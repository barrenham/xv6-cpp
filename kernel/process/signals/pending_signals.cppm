module;
#include "kernel/process/signals/signal.h"

export module xv.process:signals;

export namespace xv::process
{
inline constexpr bool supported_signal(int signal)
{
  return signal == SIGHUP || signal == SIGINT || signal == SIGKILL || signal == SIGTERM ||
         signal == SIGCONT || signal == SIGSTOP || signal == SIGTSTP ||
         signal == SIGTTIN;
}

// Standard signals coalesce. Atomic posting allows group delivery under the
// membership lock without taking a process lock or retaining a Process pointer.
class PendingSignals final
{
public:

  constexpr PendingSignals() = default;
  PendingSignals(const PendingSignals &) = delete;
  PendingSignals &operator=(const PendingSignals &) = delete;

  inline bool post(int signal)
  {
    if (!supported_signal(signal))
    {
      return false;
    }
    unsigned long bit = 1UL << signal;
    auto old = __atomic_load_n(&state_, __ATOMIC_ACQUIRE);
    for (;;)
    {
      if ((old >> 32) & bit)
      {
        return true;
      }
      auto next = old | bit;
      if (signal == SIGCONT)
      {
        next &= ~static_cast<unsigned long>(StopMask);
      }
      if (signal == SIGSTOP || signal == SIGTSTP || signal == SIGTTIN)
      {
        next &= ~(1UL << SIGCONT);
      }
      if (__atomic_compare_exchange_n(&state_, &old, next, false, __ATOMIC_ACQ_REL,
                                      __ATOMIC_ACQUIRE))
      {
        return true;
      }
    }
  }

  inline unsigned pending() const
  {
    auto value = __atomic_load_n(&state_, __ATOMIC_ACQUIRE);
    return static_cast<unsigned>(value) & ~static_cast<unsigned>(value >> 32);
  }

  // One atomic state couples disposition changes with pending-bit removal.
  inline int disposition(int signal, int action)
  {
    if ((signal != SIGHUP && signal != SIGINT && signal != SIGTERM && signal != SIGTSTP &&
         signal != SIGTTIN) ||
        (action != SIGNAL_DEFAULT && action != SIGNAL_IGNORE))
    {
      return -1;
    }
    unsigned long bit = 1UL << signal;
    auto old = __atomic_load_n(&state_, __ATOMIC_ACQUIRE);
    for (;;)
    {
      auto next =
          action == SIGNAL_IGNORE ? (old | (bit << 32)) & ~bit : old & ~(bit << 32);
      if (__atomic_compare_exchange_n(&state_, &old, next, false, __ATOMIC_ACQ_REL,
                                      __ATOMIC_ACQUIRE))
      {
        return ((old >> 32) & bit) ? SIGNAL_IGNORE : SIGNAL_DEFAULT;
      }
    }
  }

  inline void inherit_dispositions(const PendingSignals &parent)
  {
    auto state = __atomic_load_n(&parent.state_, __ATOMIC_ACQUIRE) & ~0xffffffffUL;
    __atomic_store_n(&state_, state, __ATOMIC_RELEASE);
  }

  inline int terminating_signal() const
  {
    auto bits = pending();
    if (bits & (1U << SIGKILL))
    {
      return SIGKILL;
    }
    if (bits & (1U << SIGINT))
    {
      return SIGINT;
    }
    if (bits & (1U << SIGTERM))
    {
      return SIGTERM;
    }
    if (bits & (1U << SIGHUP)) return SIGHUP;
    return 0;
  }

  static constexpr unsigned StopMask =
      (1U << SIGSTOP) | (1U << SIGTSTP) | (1U << SIGTTIN);

  inline int stopping_signal() const
  {
    auto bits = pending();
    if (bits & (1U << SIGSTOP))
    {
      return SIGSTOP;
    }
    if (bits & (1U << SIGTSTP))
    {
      return SIGTSTP;
    }
    if (bits & (1U << SIGTTIN))
    {
      return SIGTTIN;
    }
    return 0;
  }

  inline void consume_continue()
  {
    __atomic_fetch_and(&state_, ~(1UL << SIGCONT), __ATOMIC_ACQ_REL);
  }

  inline int take_stop()
  {
    auto old = __atomic_load_n(&state_, __ATOMIC_ACQUIRE);
    for (;;)
    {
      auto bits = static_cast<unsigned>(old) & ~static_cast<unsigned>(old >> 32);
      int signal = bits & (1U << SIGSTOP)   ? SIGSTOP
                   : bits & (1U << SIGTSTP) ? SIGTSTP
                   : bits & (1U << SIGTTIN) ? SIGTTIN
                                            : 0;
      if (!signal)
      {
        return 0;
      }
      auto next = old & ~static_cast<unsigned long>(StopMask);
      if (__atomic_compare_exchange_n(&state_, &old, next, false, __ATOMIC_ACQ_REL,
                                      __ATOMIC_ACQUIRE))
      {
        return signal;
      }
    }
  }

  // Only before publication or after membership removal during reap.
  inline void reset()
  {
    __atomic_store_n(&state_, 0, __ATOMIC_RELEASE);
  }

private:

  unsigned long state_ = 0;
};
} // namespace xv::process
