export module xv.trap:controller;
export import :dispatch;

export namespace xv::trap
{
class TrapController final
{
public:

  constexpr explicit TrapController(TrapDispatcher &dispatcher)
      : dispatcher_(dispatcher)
  {
  }

  TrapController(const TrapController &) = delete;
  TrapController &operator=(const TrapController &) = delete;
  void initialize_hart();
  Word handle_user();
  void handle_kernel();
  void prepare_return();
  [[noreturn]] void return_to_user();

private:

  TrapDispatcher &dispatcher_;
};

TrapController &controller();
} // namespace xv::trap
