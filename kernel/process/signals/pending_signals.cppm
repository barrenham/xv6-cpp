module;
#include "kernel/process/signals/signal_abi.h"

export module xv.process:signals;
import xv.sync;

export namespace xv::process
{
inline constexpr bool
supported_signal(int signal) {
  return signal == SIGHUP || signal == SIGINT || signal == SIGKILL ||
         signal == SIGTERM || signal == SIGCONT || signal == SIGSTOP ||
         signal == SIGTSTP || signal == SIGCHLD || signal == SIGTTIN ||
         signal == SIGUSR1 || signal == SIGUSR2 || signal == SIGSEGV ||
         signal == SIGBUS || signal == SIGPIPE;
}

// Standard signals coalesce. Atomic posting allows group delivery under the
// membership lock without taking a process lock or retaining a Process pointer.
class PendingSignals final
{
public:

  static constexpr unsigned SupportedMask =
      (1U << SIGHUP) | (1U << SIGINT) | (1U << SIGKILL) | (1U << SIGTERM) |
      (1U << SIGCONT) | (1U << SIGSTOP) | (1U << SIGTSTP) | (1U << SIGTTIN) |
      (1U << SIGPIPE) | (1U << SIGUSR1) | (1U << SIGUSR2) | (1U << SIGSEGV) |
      (1U << SIGCHLD) | (1U << SIGBUS);
  static constexpr unsigned Unmaskable = (1U << SIGKILL) | (1U << SIGSTOP);

  struct Action
  {
    unsigned long handler = 0;
    unsigned mask = 0;
    unsigned flags = 0;
    unsigned long restorer = 0;
  };

  struct Dispositions
  {
    mutable xv::sync::SpinLock lock;
    unsigned ignored = 0, handled = 0, process_pending = 0;
    int group_stop = 0;
    unsigned long action_generation = 0;
    Action actions[32]{};
    PendingSignals *members = nullptr;
  };

  constexpr PendingSignals() = default;
  PendingSignals(const PendingSignals &) = delete;
  PendingSignals &operator=(const PendingSignals &) = delete;

  inline bool
  post(int signal) {
    if (!supported_signal(signal))
    {
      return false;
    }
    xv::sync::ScopedLock guard(data().lock);
    unsigned long bit = 1UL << signal;
    auto old = __atomic_load_n(&state_, __ATOMIC_ACQUIRE);
    for (;;)
    {
      // Continue must resume a stopped task even when caught, blocked or ignored.
      auto next = ((old >> 32) & bit) && signal != SIGCONT ? old : old | bit;
      if (signal == SIGCONT)
      {
        next &= ~static_cast<unsigned long>(StopMask);
        claimed_process_ &= ~StopMask;
      }
      if (signal == SIGSTOP || signal == SIGTSTP || signal == SIGTTIN)
      {
        next &= ~(1UL << SIGCONT);
        claimed_process_ &= ~(1U << SIGCONT);
      }
      if (__atomic_compare_exchange_n(&state_, &old, next, false, __ATOMIC_ACQ_REL,
                                      __ATOMIC_ACQUIRE))
      {
        return true;
      }
    }
  }

  inline unsigned
  pending() const {
    auto value = __atomic_load_n(&state_, __ATOMIC_ACQUIRE);
    return static_cast<unsigned>(value) & ~static_cast<unsigned>(value >> 32);
  }

  inline bool
  post_process(int signal, bool queue = true) {
    if (!supported_signal(signal))
    {
      return false;
    }
    auto &table = data();
    xv::sync::ScopedLock guard(table.lock);
    unsigned bit = 1U << signal;
    auto control = [&](PendingSignals &thread) {
      if (signal == SIGCONT)
      {
        __atomic_fetch_and(&thread.state_, ~static_cast<unsigned long>(StopMask),
                           __ATOMIC_ACQ_REL);
        thread.claimed_process_ &= ~StopMask;
        __atomic_store_n(&thread.resume_, true, __ATOMIC_RELEASE);
      }
      else if (bit & StopMask)
      {
        __atomic_fetch_and(&thread.state_, ~(1UL << SIGCONT), __ATOMIC_ACQ_REL);
        thread.claimed_process_ &= ~(1U << SIGCONT);
        __atomic_store_n(&thread.resume_, false, __ATOMIC_RELEASE);
      }
    };
    if (signal == SIGCONT)
    {
      __atomic_store_n(&table.group_stop, 0, __ATOMIC_RELEASE);
      __atomic_fetch_and(&table.process_pending, ~StopMask, __ATOMIC_ACQ_REL);
    }
    else if (bit & StopMask)
    {
      __atomic_fetch_and(&table.process_pending, ~(1U << SIGCONT), __ATOMIC_ACQ_REL);
    }
    if (!table.members)
    {
      control(*this);
    }
    else
    {
      for (auto *member = table.members; member; member = member->next_)
      {
        control(*member);
      }
    }
    if (queue && !(table.ignored & bit))
    {
      __atomic_fetch_or(&table.process_pending, bit, __ATOMIC_RELEASE);
    }
    return true;
  }

  inline unsigned
  process_pending() const {
    return __atomic_load_n(&data().process_pending, __ATOMIC_ACQUIRE);
  }

  inline int
  group_stop_signal() const {
    return __atomic_load_n(&data().group_stop, __ATOMIC_ACQUIRE);
  }

  inline void
  request_group_stop(int signal) {
    if (signal <= 0 || signal >= 32 || !(StopMask & (1U << signal)))
    {
      return;
    }
    auto &table = data();
    xv::sync::ScopedLock guard(table.lock);
    if (table.group_stop || !(deliverable() & ~table.handled & (1U << signal)))
    {
      return;
    }
    __atomic_store_n(&table.group_stop, signal, __ATOMIC_RELEASE);
    __atomic_fetch_and(&state_, ~(1UL << signal), __ATOMIC_ACQ_REL);
  }

  inline bool
  deactivate() {
    auto &table = data();
    xv::sync::ScopedLock guard(table.lock);
    __atomic_store_n(&active_, false, __ATOMIC_RELEASE);
    // Selection by a wakeup probe is not delivery. A process-directed caught
    // signal must survive its selected thread exiting before entering a handler.
    unsigned retry = claimed_process_ & pending() & ~table.ignored;
    claimed_process_ = 0;
    __atomic_fetch_and(&state_, ~0UL << 32, __ATOMIC_ACQ_REL);
    __atomic_fetch_or(&table.process_pending, retry, __ATOMIC_RELEASE);
    return retry != 0;
  }

  inline void
  clear_resume() {
    __atomic_store_n(&resume_, false, __ATOMIC_RELEASE);
  }

  // One atomic state couples disposition changes with pending-bit removal.
  inline int
  disposition(int signal, int action) {
    if (!supported_signal(signal) || signal == SIGKILL || signal == SIGSTOP ||
        (action != SIGNAL_DEFAULT && action != SIGNAL_IGNORE))
    {
      return -1;
    }
    auto &table = data();
    xv::sync::ScopedLock guard(table.lock);
    int old = table.ignored & (1U << signal) ? SIGNAL_IGNORE : SIGNAL_DEFAULT;
    install_locked(signal, {static_cast<unsigned long>(action), 0, 0, 0});
    return old;
  }

  inline void
  inherit_dispositions(const PendingSignals &parent) {
    Action actions[32];
    {
      auto &source = parent.data();
      xv::sync::ScopedLock guard(source.lock);
      for (unsigned i = 0; i < 32; ++i)
      {
        actions[i] = source.actions[i];
      }
    }
    {
      xv::sync::ScopedLock guard(data().lock);
      for (unsigned i = 1; i < 32; ++i)
      {
        install_locked(i, actions[i]);
      }
    }
    set_mask(parent.mask());
  }

  // The initial task owns the table until the whole process is reaped. Its
  // descriptor stays alive even after pthread_exit, so bindings remain stable.
  inline void
  share_dispositions(PendingSignals &parent) {
    if (this == &parent || __atomic_load_n(&shared_, __ATOMIC_ACQUIRE))
    {
      __builtin_trap();
    }
    auto &table = parent.data();
    xv::sync::ScopedLock guard(table.lock);
    if (!__atomic_load_n(&parent.shared_, __ATOMIC_ACQUIRE))
    {
      parent.next_ = table.members;
      table.members = &parent;
      __atomic_store_n(&parent.shared_, &table, __ATOMIC_RELEASE);
    }
    next_ = table.members;
    table.members = this;
    __atomic_store_n(&shared_, &table, __ATOMIC_RELEASE);
    __atomic_store_n(&state_, static_cast<unsigned long>(table.ignored) << 32,
                     __ATOMIC_RELEASE);
    set_mask(parent.mask());
  }

  inline unsigned
  mask() const {
    return __atomic_load_n(&blocked_, __ATOMIC_ACQUIRE);
  }

  inline unsigned
  handled() const {
    return __atomic_load_n(&data().handled, __ATOMIC_ACQUIRE);
  }

  inline void
  set_mask(unsigned value) {
    __atomic_store_n(&blocked_, value & SupportedMask & ~Unmaskable, __ATOMIC_RELEASE);
  }

  inline unsigned
  deliverable() const {
    return pending() & ~mask();
  }

  inline bool
  interrupting() const {
    return terminating_signal() || stopping_signal() || caught_signal();
  }

  inline int
  caught_signal() const {
    claim_process();
    auto &table = data();
    xv::sync::ScopedLock guard(table.lock);
    unsigned bits = deliverable() & table.handled;
    return bits ? __builtin_ctz(bits) : 0;
  }

  inline bool
  continuing() const {
    return __atomic_load_n(&resume_, __ATOMIC_ACQUIRE) ||
           (__atomic_load_n(&state_, __ATOMIC_ACQUIRE) & (1UL << SIGCONT));
  }

  inline void
  consume(int signal) {
    xv::sync::ScopedLock guard(data().lock);
    __atomic_fetch_and(&state_, ~(1UL << signal), __ATOMIC_ACQ_REL);
    claimed_process_ &= ~(1U << signal);
  }

  inline Action
  action(int signal, unsigned long *generation = nullptr) const {
    auto &table = data();
    xv::sync::ScopedLock guard(table.lock);
    if (generation)
    {
      *generation = table.action_generation;
    }
    return table.actions[signal];
  }

  inline void
  set_action(int signal, Action value) {
    xv::sync::ScopedLock guard(data().lock);
    install_locked(signal, value);
  }

  inline bool
  set_action_if(int signal, Action value, unsigned long generation) {
    auto &table = data();
    xv::sync::ScopedLock guard(table.lock);
    if (generation != table.action_generation)
    {
      return false;
    }
    install_locked(signal, value);
    return true;
  }

  inline bool
  take_handler(int &signal, Action &action) {
    claim_process();
    auto &table = data();
    xv::sync::ScopedLock guard(table.lock);
    auto bits = deliverable() & table.handled;
    if (!bits)
    {
      return false;
    }
    signal = __builtin_ctz(bits);
    action = table.actions[signal];
    claimed_process_ &= ~(1U << signal);
    __atomic_fetch_and(&state_, ~(1UL << signal), __ATOMIC_ACQ_REL);
    if (action.flags & SA_RESETHAND)
    {
      install_locked(signal, {});
    }
    return true;
  }

  inline void
  on_exec() {
    auto &table = data();
    xv::sync::ScopedLock guard(table.lock);
    auto bits = table.handled;
    for (int i = 1; i < 32; ++i)
    {
      if (bits & (1U << i))
      {
        install_locked(i, {});
      }
    }
  }

  inline int
  terminating_signal() const {
    claim_process();
    auto &table = data();
    xv::sync::ScopedLock guard(table.lock);
    auto bits = deliverable() & ~table.handled;
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
    if (bits & (1U << SIGHUP))
    {
      return SIGHUP;
    }
    if (bits & (1U << SIGUSR1))
    {
      return SIGUSR1;
    }
    if (bits & (1U << SIGUSR2))
    {
      return SIGUSR2;
    }
    if (bits & (1U << SIGSEGV))
    {
      return SIGSEGV;
    }
    if (bits & (1U << SIGBUS))
    {
      return SIGBUS;
    }
    if (bits & (1U << SIGPIPE))
    {
      return SIGPIPE;
    }
    return 0;
  }

  static constexpr unsigned StopMask =
      (1U << SIGSTOP) | (1U << SIGTSTP) | (1U << SIGTTIN);

  inline int
  stopping_signal() const {
    claim_process();
    auto &table = data();
    xv::sync::ScopedLock guard(table.lock);
    if (auto signal = group_stop_signal())
    {
      return signal;
    }
    auto bits = deliverable() & ~table.handled;
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

  inline void
  consume_continue() {
    clear_resume();
    __atomic_fetch_and(&state_, ~(1UL << SIGCONT), __ATOMIC_ACQ_REL);
  }

  inline int
  take_stop() {
    claim_process();
    xv::sync::ScopedLock guard(data().lock);
    if (auto signal = group_stop_signal())
    {
      return signal;
    }
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
  inline void
  reset() {
    if (auto *table = __atomic_load_n(&shared_, __ATOMIC_ACQUIRE))
    {
      xv::sync::ScopedLock guard(table->lock);
      auto **member = &table->members;
      while (*member && *member != this)
      {
        member = &(*member)->next_;
      }
      if (*member)
      {
        *member = next_;
      }
      __atomic_store_n(&shared_, static_cast<Dispositions *>(nullptr),
                       __ATOMIC_RELEASE);
      next_ = nullptr;
    }
    // Only unpublished/retired tasks reset. A table owner is reaped last.
    if (local_.members)
    {
      __builtin_trap();
    }
    local_.ignored = local_.handled = local_.process_pending = 0;
    local_.group_stop = 0;
    local_.action_generation = 0;
    claimed_process_ = 0;
    active_ = true;
    resume_ = false;
    for (auto &action : local_.actions)
    {
      action = {};
    }
    __atomic_store_n(&state_, 0, __ATOMIC_RELEASE);
    __atomic_store_n(&blocked_, 0, __ATOMIC_RELEASE);
  }

private:

  mutable unsigned long state_ = 0;
  mutable unsigned claimed_process_ = 0;
  unsigned blocked_ = 0;
  mutable Dispositions local_;
  Dispositions *shared_ = nullptr;
  PendingSignals *next_ = nullptr;
  mutable bool active_ = true, resume_ = false;

  Dispositions &
  data() const {
    auto *shared = __atomic_load_n(&shared_, __ATOMIC_ACQUIRE);
    return shared ? *shared : local_;
  }

  void
  claim_process() const {
    if (!__atomic_load_n(&active_, __ATOMIC_ACQUIRE) || !process_pending())
    {
      return;
    }
    auto &table = data();
    xv::sync::ScopedLock guard(table.lock);
    if (!__atomic_load_n(&active_, __ATOMIC_ACQUIRE))
    {
      return;
    }
    unsigned eligible = table.process_pending & ~mask() & ~table.ignored;
    auto stops = eligible & StopMask & ~table.handled;
    if (stops && !table.group_stop)
    {
      int signal = stops & (1U << SIGSTOP)   ? SIGSTOP
                   : stops & (1U << SIGTSTP) ? SIGTSTP
                                             : SIGTTIN;
      __atomic_store_n(&table.group_stop, signal, __ATOMIC_RELEASE);
    }
    claimed_process_ |= eligible & table.handled;
    __atomic_fetch_and(&table.process_pending, ~eligible, __ATOMIC_ACQ_REL);
    __atomic_fetch_or(&state_, static_cast<unsigned long>(eligible), __ATOMIC_ACQ_REL);
  }

  void
  install_locked(int signal, Action value) {
    auto &table = data();
    unsigned bit = 1U << signal;
    bool ignored = value.handler == SIGNAL_IGNORE;
    if (ignored)
    {
      table.ignored |= bit;
    }
    else
    {
      table.ignored &= ~bit;
    }
    if (ignored || (signal == SIGCHLD && value.handler == SIGNAL_DEFAULT))
    {
      __atomic_fetch_and(&table.process_pending, ~bit, __ATOMIC_ACQ_REL);
    }
    table.actions[signal] = value;
    ++table.action_generation;
    if (value.handler > SIGNAL_IGNORE)
    {
      __atomic_fetch_or(&table.handled, bit, __ATOMIC_RELEASE);
    }
    else
    {
      __atomic_fetch_and(&table.handled, ~bit, __ATOMIC_RELEASE);
    }
    auto update = [&](PendingSignals &thread) {
      auto old = __atomic_load_n(&thread.state_, __ATOMIC_ACQUIRE);
      for (;;)
      {
        auto next = (old & ~(static_cast<unsigned long>(bit) << 32)) |
                    (ignored ? static_cast<unsigned long>(bit) << 32 : 0UL);
        if (ignored || (signal == SIGCHLD && value.handler == SIGNAL_DEFAULT))
        {
          next &= ~static_cast<unsigned long>(bit);
          thread.claimed_process_ &= ~bit;
        }
        if (__atomic_compare_exchange_n(&thread.state_, &old, next, false,
                                        __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))
        {
          break;
        }
      }
    };
    if (!table.members)
    {
      update(*this);
    }
    else
    {
      for (auto *thread = table.members; thread; thread = thread->next_)
      {
        update(*thread);
      }
    }
  }
};
} // namespace xv::process
