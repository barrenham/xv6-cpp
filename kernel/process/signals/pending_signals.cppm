module;
#include "kernel/process/signals/signal.h"

export module xv.process:signals;

export namespace xv::process
{
inline constexpr bool supported_signal(int signal)
{
  return signal == SIGHUP || signal == SIGINT || signal == SIGKILL || signal == SIGTERM ||
         signal == SIGCONT || signal == SIGSTOP || signal == SIGTSTP ||
         signal == SIGCHLD || signal == SIGTTIN || signal == SIGUSR1 || signal == SIGUSR2 || signal == SIGSEGV;
}

// Standard signals coalesce. Atomic posting allows group delivery under the
// membership lock without taking a process lock or retaining a Process pointer.
class PendingSignals final
{
public:

  static constexpr unsigned SupportedMask =
      (1U << SIGHUP) | (1U << SIGINT) | (1U << SIGKILL) | (1U << SIGTERM) |
      (1U << SIGCONT) | (1U << SIGSTOP) | (1U << SIGTSTP) | (1U << SIGTTIN) |
      (1U << SIGUSR1) | (1U << SIGUSR2) | (1U << SIGSEGV) | (1U << SIGCHLD);
  static constexpr unsigned Unmaskable = (1U << SIGKILL) | (1U << SIGSTOP);
  struct Action
  {
    unsigned long handler = 0;
    unsigned mask = 0;
    unsigned flags = 0;
    unsigned long restorer = 0;
  };
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
      // Continue must resume a stopped task even when caught, blocked or ignored.
      auto next = ((old >> 32) & bit) && signal != SIGCONT ? old : old | bit;
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
    if (!supported_signal(signal) || signal == SIGKILL || signal == SIGSTOP ||
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
      if (signal == SIGCHLD && action == SIGNAL_DEFAULT) next &= ~bit;
      if (__atomic_compare_exchange_n(&state_, &old, next, false, __ATOMIC_ACQ_REL,
                                      __ATOMIC_ACQUIRE))
      {
        __atomic_fetch_and(&handled_, ~static_cast<unsigned>(bit), __ATOMIC_RELEASE);
        actions_[signal] = {};
        return ((old >> 32) & bit) ? SIGNAL_IGNORE : SIGNAL_DEFAULT;
      }
    }
  }

  inline void inherit_dispositions(const PendingSignals &parent)
  {
    auto state = __atomic_load_n(&parent.state_, __ATOMIC_ACQUIRE) & ~0xffffffffUL;
    __atomic_store_n(&state_, state, __ATOMIC_RELEASE);
    set_mask(parent.mask());
    __atomic_store_n(&handled_, parent.handled(), __ATOMIC_RELEASE);
    for (int i = 0; i < 32; ++i) actions_[i] = parent.actions_[i];
  }

  inline unsigned mask() const { return __atomic_load_n(&blocked_, __ATOMIC_ACQUIRE); }
  inline unsigned handled() const { return __atomic_load_n(&handled_, __ATOMIC_ACQUIRE); }
  inline void set_mask(unsigned value)
  {
    __atomic_store_n(&blocked_, value & SupportedMask & ~Unmaskable, __ATOMIC_RELEASE);
  }
  inline unsigned deliverable() const { return pending() & ~mask(); }
  inline bool interrupting() const
  {
    return terminating_signal() || stopping_signal() || caught_signal();
  }
  inline int caught_signal() const
  {
    unsigned bits = deliverable() & handled();
    return bits ? __builtin_ctz(bits) : 0;
  }
  inline bool continuing() const
  {
    return __atomic_load_n(&state_, __ATOMIC_ACQUIRE) & (1UL << SIGCONT);
  }
  inline void consume(int signal)
  {
    __atomic_fetch_and(&state_, ~(1UL << signal), __ATOMIC_ACQ_REL);
  }
  inline Action action(int signal) const
  {
    auto result = actions_[signal];
    if (!(__atomic_load_n(&state_, __ATOMIC_ACQUIRE) & (1UL << (signal + 32))))
      return result;
    result.handler = SIGNAL_IGNORE;
    return result;
  }
  inline void set_action(int signal, Action value)
  {
    if (value.handler <= SIGNAL_IGNORE)
    {
      disposition(signal, value.handler == SIGNAL_IGNORE ? SIGNAL_IGNORE : SIGNAL_DEFAULT);
    }
    else
    {
      __atomic_fetch_and(&state_, ~(1UL << (signal + 32)), __ATOMIC_ACQ_REL);
    }
    actions_[signal] = value;
    if (value.handler > SIGNAL_IGNORE)
      __atomic_fetch_or(&handled_, 1U << signal, __ATOMIC_RELEASE);
  }
  inline void on_exec()
  {
    for (int i = 1; i < 32; ++i)
      if (handled() & (1U << i)) set_action(i, {});
  }

  inline int terminating_signal() const
  {
    auto bits = deliverable() & ~handled();
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
    if (bits & (1U << SIGUSR1)) return SIGUSR1;
    if (bits & (1U << SIGUSR2)) return SIGUSR2;
    if (bits & (1U << SIGSEGV)) return SIGSEGV;
    return 0;
  }

  static constexpr unsigned StopMask =
      (1U << SIGSTOP) | (1U << SIGTSTP) | (1U << SIGTTIN);

  inline int stopping_signal() const
  {
    auto bits = deliverable() & ~handled();
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
      auto bits = static_cast<unsigned>(old) & ~static_cast<unsigned>(old >> 32) &
                  ~mask() & ~handled();
      int signal = bits & (1U << SIGSTOP)   ? SIGSTOP
                   : bits & (1U << SIGTSTP) ? SIGTSTP
                   : bits & (1U << SIGTTIN) ? SIGTTIN
                                            : 0;
      if (!signal)
      {
        return 0;
      }
      auto next = old & ~(1UL << signal);
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
    __atomic_store_n(&blocked_, 0, __ATOMIC_RELEASE);
    __atomic_store_n(&handled_, 0, __ATOMIC_RELEASE);
    for (auto &value : actions_) value = {};
  }

private:

  unsigned long state_ = 0;
  unsigned blocked_ = 0;
  unsigned handled_ = 0;
  Action actions_[32]{};
};
} // namespace xv::process
