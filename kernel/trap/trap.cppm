export module xv.trap;
export import :event;
export import :dispatch;
export import :clock;
export import :controller;

export namespace xv::trap
{
void initialize();
void initialize_hart();
void prepare_return();
[[noreturn]] void return_to_user();
} // namespace xv::trap
