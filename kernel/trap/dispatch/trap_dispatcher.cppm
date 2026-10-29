export module xv.trap:dispatch;
export import :event;

export namespace xv::trap
{
class TrapHandler
{
public:

  TrapHandler(const TrapHandler &) = delete;
  TrapHandler &operator=(const TrapHandler &) = delete;
  virtual const char *name() const = 0;
  virtual bool accepts(const TrapEvent &event) const = 0;
  virtual TrapAction handle(const TrapEvent &event) = 0;

protected:

  constexpr TrapHandler() = default;
  ~TrapHandler() = default;
};

// Fixed, non-owning handler table. The first accepting handler is selected.
class TrapDispatcher final
{
public:

  constexpr TrapDispatcher(TrapHandler *const *handlers, unsigned count)
      : handlers_(handlers), count_(count)
  {
  }

  TrapDispatcher(const TrapDispatcher &) = delete;
  TrapDispatcher &operator=(const TrapDispatcher &) = delete;
  TrapAction dispatch(const TrapEvent &event);

private:

  struct DispatchFlow;

  TrapHandler *const *handlers_;
  unsigned count_;
};
} // namespace xv::trap
