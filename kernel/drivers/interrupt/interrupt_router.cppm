export module xv.drivers:router;

export import :interrupt;

export namespace xv::drivers
{
// Non-owning fixed board wiring. Controller, table and devices must outlive it.
class InterruptRouter final
{
public:

  constexpr InterruptRouter(InterruptController &controller,
                            InterruptDevice *const *devices, unsigned count)
      : controller_(controller), devices_(devices), count_(count)
  {
  }

  InterruptRouter(const InterruptRouter &) = delete;
  InterruptRouter &operator=(const InterruptRouter &) = delete;

  void initialize();
  void initialize_hart(int hart);
  // Returns an unknown IRQ for diagnostics, or zero for known/spurious IRQs.
  // Every non-zero claim is completed after its handler returns.
  unsigned dispatch(int hart);

private:

  struct InitializeFlow;
  struct DispatchFlow;

  InterruptController &controller_;
  InterruptDevice *const *devices_;
  unsigned count_;
};
} // namespace xv::drivers
