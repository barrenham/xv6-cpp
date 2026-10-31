module;

#include "kernel/support/types.h"
#include "kernel/support/flow.h"
#include "kernel/support/param.h"
#include "kernel/memory/layout/memlayout.h"
#include "kernel/arch/riscv/register_constants.h"

module xv.process;

import xv.runtime;
import :manager;

namespace xv::process
{

namespace flow = xv::flow;

struct Process::CommitFlow
{
  struct Context
  {
    Process &owner;
    AddressSpace &image;
    unsigned long entry, stack, arguments;
    const char *path;
    unsigned long environment;
    ExecSnapshot *snapshot;
  };

  static void linearize(Context &c) noexcept
  {
    process_manager.did_exec(c.owner);
  }

  static void image(Context &c) noexcept
  {
    c.owner.memory_.swap(c.image);
  }

  static void signals(Context &c) noexcept
  {
    c.owner.pending_signals_.on_exec();
    c.owner.signal_depth_ = 0;
    c.owner.suspended_mask_ = c.owner.suspend_restarted_ = false;
  }

  static void registers(Context &c) noexcept
  {
    c.owner.trapframe->epc = c.entry;
    c.owner.trapframe->sp = c.stack;
    c.owner.trapframe->a1 = c.arguments;
    c.owner.trapframe->a2 = c.environment;
  }

  static void name(Context &c) noexcept
  {
    const char *last = c.path;
    for (auto *p = c.path; *p; ++p)
    {
      if (*p == '/')
      {
        last = p + 1;
      }
    }
    xv::sync::ScopedLock guard(c.owner.lock);
    xv::runtime::copy_string(c.owner.name,last,sizeof(c.owner.name));
    if(c.snapshot)c.owner.exec_snapshot_.swap(*c.snapshot);
  }
  struct Linearize;
  struct Image;
  struct Signals;
  struct Registers;
  struct Name;
  using Graph = flow::graph<
      Linearize,
      flow::types<flow::node<Linearize, flow::action<&linearize>, flow::go<Image>>,
                  flow::node<Image, flow::action<&image>, flow::go<Signals>>,
                  flow::node<Signals, flow::action<&signals>, flow::go<Registers>>,
                  flow::node<Registers, flow::action<&registers>, flow::go<Name>>,
                  flow::node<Name, flow::action<&name>, flow::stop>>>;
};

void Process::set_name(const char *label)
{
  xv::sync::ScopedLock guard(lock);
  xv::runtime::copy_string(name, label, sizeof(name));
}

bool Process::create_image(AddressSpace &image)
{
  return image.create(trapframe, usyscall);
}

void Process::commit_image(AddressSpace &image, unsigned long entry,
                           unsigned long stack, unsigned long arguments,
                           const char *path, unsigned long environment,ExecSnapshot *snapshot)
{
  CommitFlow::Context context{*this, image, entry, stack, arguments, path, environment,snapshot};
  (void)CommitFlow::Graph::run<0>(context);
}
} // namespace xv::process
