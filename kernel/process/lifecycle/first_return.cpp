module;
#include "kernel/support/param.h"
#include "kernel/support/flow.h"

module xv.process;

import xv.fs;
import xv.trap;
import xv.print;
import xv.memory;
import xv.drivers;
import xv.net;

namespace xv::process
{

namespace
{
void
initialize_network() {
  xv::net::initialize();
  xv::print::printk("network: %s\n",
                    xv::drivers::network_device().online() ? "ready" : "not present");
  for (unsigned i = 0; i < xv::net::interfaces().count(); ++i)
  {
    auto *interface = xv::net::interfaces().at(i);
    xv::print::printk("net: %s mtu=%d %s\n", interface->name(), interface->mtu(),
                      interface->up() && interface->carrier() ? "up" : "down");
    for (unsigned n = 0; n < interface->address_count(); ++n)
    {
      auto address = interface->address(n);
      xv::print::printk("net: %s %d.%d.%d.%d/%d\n", interface->name(),
                        address.address >> 24, (address.address >> 16) & 255,
                        (address.address >> 8) & 255, address.address & 255,
                        address.prefix);
    }
  }
  xv::net::start_timer_worker();
}
} // namespace

namespace flow = xv::flow;

struct Process::FirstReturnFlow
{
  struct Context
  {
    Process &owner;
    int &first;
  };

  static void
  unlock(Context &c) noexcept {
    c.owner.lock.release();
  }

  static bool
  first(Context &c) noexcept {
    return __atomic_load_n(&c.first, __ATOMIC_ACQUIRE);
  }

  static void
  initialize(Context &) noexcept {
    if (xv::memory::boot_image().empty())
    {
      xv::print::panic("boot: missing userspace bootstrap (-initrd)");
    }
  }

  static void
  publish(Context &c) noexcept {
    __atomic_store_n(&c.first, 0, __ATOMIC_RELEASE);
  }

  static void
  execute_init(Context &c) noexcept {
    auto image = xv::memory::boot_image();
    if (!image.empty())
    {
      xv::drivers::initialize_boot_modules();
      initialize_network();
      xv::fs::initialize_device_registry();
      // Bootstrap console descriptors require neither /dev nor a root directory.
      for (int descriptor = 0; descriptor < 3; ++descriptor)
      {
        auto *file = xv::fs::files().allocate();
        if (!file)
        {
          xv::print::panic("boot console: no file slot");
        }
        xv::fs::files().bind_endpoint(*file, xv::drivers::console());
        if (c.owner.files().install(file) != descriptor)
        {
          xv::fs::files().release(file);
          xv::print::panic("boot console: descriptor allocation");
        }
      }
      auto *disk = xv::fs::create_boot_disk(0);
      if (!disk || c.owner.files().install(disk) != 3)
      {
        if (disk)
        {
          xv::fs::files().release(disk);
        }
        xv::print::panic("boot disk: descriptor allocation");
      }
      auto *resources = xv::fs::create_fs_resources();
      if (!resources || c.owner.files().install(resources) != 4)
      {
        if (resources)
        {
          xv::fs::files().release(resources);
        }
        xv::print::panic("boot resources: descriptor allocation");
      }
      char *arguments[] = {const_cast<char *>("/boot/fsboot"), nullptr};
      c.owner.trapframe->a0 = executable_loader().load_memory(
          c.owner, image.begin().pointer(), image.size(), arguments[0], arguments,
          nullptr);
      return;
    }
  }

  static bool
  loaded(Context &c) noexcept {
    return c.owner.trapframe->a0 != -1;
  }

  static void
  fail(Context &) noexcept {
    xv::print::panic("exec");
  }

  static void
  user(Context &) noexcept {
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
void
first_return() {
  static int first = 1;
  Process::FirstReturnFlow::Context context{*current(), first};
  (void)Process::FirstReturnFlow::Graph::run<0>(context);
}

} // namespace xv::process
