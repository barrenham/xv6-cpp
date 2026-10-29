module;
#include "kernel/support/flow.h"
#include "kernel/net/errno.h"
module xv.net:interface_update_flows;
import :interface;

namespace xv::net
{
namespace flow = xv::flow;

struct InterfaceRegistry::UpdateFlow
{
  struct Context
  {
    InterfaceRegistry &owner;
    NetworkInterface &interface;
    unsigned index = 0;
    int result = 0;
    bool success = false;
  };

  static inline bool sealed(Context &c) noexcept
  {
    return c.owner.sealed_;
  }

  static inline void validate(Context &c) noexcept
  {
    c.result = c.owner.registration_error(c.interface);
  }

  static inline bool valid(Context &c) noexcept
  {
    return c.result == 0;
  }

  static inline bool nonnegative(Context &c) noexcept
  {
    return c.result >= 0;
  }

  static inline void publish(Context &c) noexcept
  {
    c.interface.sealed_ = true;
    ++c.interface.generation_;
  }

  static inline void append(Context &c) noexcept
  {
    c.owner.interfaces_[c.owner.count_++] = &c.interface;
    c.success = true;
  }

  static inline bool more(Context &c) noexcept
  {
    return c.index < c.owner.count_;
  }

  static inline bool matches(Context &c) noexcept
  {
    return c.owner.interfaces_[c.index] == &c.interface;
  }

  static inline void erase(Context &c) noexcept
  {
    for (unsigned next = c.index + 1; next < c.owner.count_; ++next)
    {
      c.owner.interfaces_[next - 1] = c.owner.interfaces_[next];
    }
    c.owner.interfaces_[--c.owner.count_] = nullptr;
    ++c.interface.generation_;
    c.success = true;
  }

  static inline void advance(Context &c) noexcept
  {
    ++c.index;
  }
  struct AddSealed;
  struct AddValidate;
  struct AddCommit;
  using AddGraph = flow::graph<
      AddSealed,
      flow::types<flow::node<AddSealed, flow::noop,
                             flow::branch<flow::condition<&sealed>, flow::stop,
                                          flow::go<AddValidate>>>,
                  flow::node<AddValidate, flow::action<&validate>,
                             flow::branch<flow::condition<&valid>, flow::go<AddCommit>,
                                          flow::stop>>,
                  flow::node<AddCommit, flow::action<&append>, flow::stop>>>;
  struct PublishValidate;
  struct PublishSeal;
  struct PublishCommit;
  using PublishGraph = flow::graph<
      PublishValidate,
      flow::types<
          flow::node<PublishValidate, flow::action<&validate>,
                     flow::branch<flow::condition<&nonnegative>, flow::go<PublishSeal>,
                                  flow::stop>>,
          flow::node<PublishSeal, flow::action<&publish>, flow::go<PublishCommit>>,
          flow::node<PublishCommit, flow::action<&append>, flow::stop>>>;
  struct WithdrawScan;
  struct WithdrawMatch;
  struct WithdrawErase;
  struct WithdrawAdvance;
  using WithdrawGraph = flow::graph<
      WithdrawScan,
      flow::types<
          flow::node<WithdrawScan, flow::noop,
                     flow::branch<flow::condition<&more>, flow::go<WithdrawMatch>,
                                  flow::stop>>,
          flow::node<WithdrawMatch, flow::noop,
                     flow::branch<flow::condition<&matches>, flow::go<WithdrawErase>,
                                  flow::go<WithdrawAdvance>>>,
          flow::node<WithdrawErase, flow::action<&erase>, flow::stop>,
          flow::node<WithdrawAdvance, flow::action<&advance>, flow::go<WithdrawScan>>>>;
};

struct InterfaceRegistry::SealFlow
{
  struct Context
  {
    InterfaceRegistry &owner;
    unsigned interface = 0, other = 0, address = 0;
    bool result = false;
  };

  static inline bool more_interfaces(Context &c) noexcept
  {
    return c.interface < c.owner.count_;
  }

  static inline bool more_others(Context &c) noexcept
  {
    return c.other < c.owner.count_;
  }

  static inline bool distinct(Context &c) noexcept
  {
    return c.interface != c.other;
  }

  static inline void first_address(Context &c) noexcept
  {
    c.address = 0;
  }

  static inline bool more_addresses(Context &c) noexcept
  {
    return c.address < c.owner.interfaces_[c.interface]->address_count();
  }

  static inline bool conflict(Context &c) noexcept
  {
    return c.owner.interfaces_[c.other]->owns(
        c.owner.interfaces_[c.interface]->address(c.address).address);
  }

  static inline void next_address(Context &c) noexcept
  {
    ++c.address;
  }

  static inline void next_other(Context &c) noexcept
  {
    ++c.other;
  }

  static inline void next_interface(Context &c) noexcept
  {
    ++c.interface;
    c.other = 0;
  }

  static inline void commit(Context &c) noexcept
  {
    c.owner.sealed_ = true;
    for (unsigned i = 0; i < c.owner.count_; ++i)
    {
      c.owner.interfaces_[i]->sealed_ = true;
    }
    c.result = true;
  }
  struct Interfaces;
  struct Others;
  struct Distinct;
  struct FirstAddress;
  struct Addresses;
  struct Conflict;
  struct NextAddress;
  struct NextOther;
  struct NextInterface;
  struct Commit;
  using Graph = flow::graph<
      Interfaces,
      flow::types<
          flow::node<Interfaces, flow::noop,
                     flow::branch<flow::condition<&more_interfaces>, flow::go<Others>,
                                  flow::go<Commit>>>,
          flow::node<Others, flow::noop,
                     flow::branch<flow::condition<&more_others>, flow::go<Distinct>,
                                  flow::go<NextInterface>>>,
          flow::node<Distinct, flow::noop,
                     flow::branch<flow::condition<&distinct>, flow::go<FirstAddress>,
                                  flow::go<NextOther>>>,
          flow::node<FirstAddress, flow::action<&first_address>, flow::go<Addresses>>,
          flow::node<Addresses, flow::noop,
                     flow::branch<flow::condition<&more_addresses>, flow::go<Conflict>,
                                  flow::go<NextOther>>>,
          flow::node<Conflict, flow::noop,
                     flow::branch<flow::condition<&conflict>, flow::stop,
                                  flow::go<NextAddress>>>,
          flow::node<NextAddress, flow::action<&next_address>, flow::go<Addresses>>,
          flow::node<NextOther, flow::action<&next_other>, flow::go<Others>>,
          flow::node<NextInterface, flow::action<&next_interface>,
                     flow::go<Interfaces>>,
          flow::node<Commit, flow::action<&commit>, flow::stop>>>;
};

} // namespace xv::net
