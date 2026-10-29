module;
#include "kernel/support/param.h"
#include "kernel/support/flow.h"
#include "kernel/fs/image_paths.h"

module xv.process;

import xv.fs;
import xv.trap;
import xv.print;

namespace xv::process
{

namespace flow = xv::flow;

struct Process::FirstReturnFlow
{
  struct Context
  {
    Process &owner;
    int &first;
  };

  static void unlock(Context &c) noexcept
  {
    c.owner.lock.release();
  }

  static bool first(Context &c) noexcept
  {
    return __atomic_load_n(&c.first, __ATOMIC_ACQUIRE);
  }

  static void initialize(Context &) noexcept
  {
    xv::fs::filesystem().initialize(ROOTDEV);
  }

  static void publish(Context &c) noexcept
  {
    __atomic_store_n(&c.first, 0, __ATOMIC_RELEASE);
  }

  static void execute_init(Context &c) noexcept
  {
    char *arguments[] = {const_cast<char *>(XV_INIT_PATH), nullptr};
    c.owner.trapframe->a0 = execute(XV_INIT_PATH, arguments);
  }

  static bool loaded(Context &c) noexcept
  {
    return c.owner.trapframe->a0 != -1;
  }

  static void fail(Context &) noexcept
  {
    xv::print::panic("exec");
  }

  static void user(Context &) noexcept
  {
    xv::trap::return_to_user();
  }
  struct Unlock;
  struct Initialize;
  struct Publish;
  struct Execute;
  struct Fail;
  struct User;
  using Graph = flow::graph<
      Unlock,
      flow::types<flow::node<Unlock, flow::action<&unlock>,
                             flow::branch<flow::condition<&first>, flow::go<Initialize>,
                                          flow::go<User>>>,
                  flow::node<Initialize, flow::action<&initialize>, flow::go<Publish>>,
                  flow::node<Publish, flow::action<&publish>, flow::go<Execute>>,
                  flow::node<Execute, flow::action<&execute_init>,
                             flow::branch<flow::condition<&loaded>, flow::go<User>,
                                          flow::go<Fail>>>,
                  flow::node<Fail, flow::action<&fail>, flow::stop>,
                  flow::node<User, flow::action<&user>, flow::stop>>>;
};

// swtch enters here only on a process's first dispatch, holding its process lock.
void first_return()
{
  static int first = 1;
  Process::FirstReturnFlow::Context context{*current(), first};
  (void)Process::FirstReturnFlow::Graph::run<0>(context);
}

} // namespace xv::process
