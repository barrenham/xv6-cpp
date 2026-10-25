module;

#include <cstdio>
#include <functional>
#include "kernel/tty/termios.h"
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <type_traits>
#include <sys/wait.h>
#include <unistd.h>

module xv.drivers;

import :platform;
import :tty;
import :n_tty;
import :uart;
import :plic;
import :virtio;
import :protocol;

namespace xv::drivers
{
namespace test
{
static_assert(std::is_abstract_v<Device>);
static_assert(std::is_abstract_v<SerialPort>);
static_assert(std::is_abstract_v<BlockDevice>);
static_assert(std::is_abstract_v<InterruptController>);
static_assert(!std::is_copy_constructible_v<VirtioBlock>);
static_assert(!std::is_destructible_v<Device>);

void check(bool condition, const char *message)
{
  if (!condition)
  {
    std::fprintf(stderr, "FAIL: %s\n", message);
    std::exit(1);
  }
}

template <class Operation> void expect_panic(Operation operation)
{
  auto child = fork();
  check(child >= 0, "fork");
  if (child == 0)
  {
    operation();
    std::_Exit(0);
  }
  int status;
  check(waitpid(child, &status, 0) == child, "waitpid");
  check(WIFEXITED(status) && WEXITSTATUS(status) == 99, "invalid device operation");
}

std::function<void()> on_sleep;
unsigned long milliseconds = 0;
bool killed = false;
bool fail_copy = false;
bool copied_user = false;
unsigned wakeups = 0;
unsigned dumps = 0;
unsigned interrupts = 0;
std::vector<void *> dma_pages;

class FakeSerial final : public SerialPort
{
public:

  unsigned drains = 0;

  void drain() override
  {
    ++drains;
  }

  std::string output;
  InputReceiver *receiver = nullptr;

  const char *name() const override
  {
    return "test serial";
  }

  void initialize() override
  {
  }

  unsigned irq() const override
  {
    return 10;
  }

  void handle_interrupt() override
  {
  }

  void attach(InputReceiver &input) override
  {
    receiver = &input;
  }

  void write(const char *data, int count) override
  {
    output.append(data, count);
  }

  void put_sync(int character) override
  {
    output.push_back(character);
  }
};

void console_contract()
{
  FakeSerial serial;
  NttyDiscipline discipline;
  Tty console(serial, discipline);
  console.initialize();
  char input[256]{};
  auto buffer = [](void *p)
  { return IoBuffer{AddressSpace::Kernel, reinterpret_cast<unsigned long>(p)}; };
  auto destination = buffer(input);
  termios settings{};
  check(console.ioctl(TCGETS, buffer(&settings)) == 0, "get termios");
  auto defaults = settings;
  auto set = [&]
  { check(console.ioctl(TCSETS, buffer(&settings)) == 0, "set termios"); };
  auto feed = [&](const std::string &text)
  {
    for (unsigned char c : text)
    {
      console.receive(c);
    }
  };
  auto queued = [&]
  {
    int n = -1;
    check(console.ioctl(FIONREAD, buffer(&n)) == 0, "FIONREAD");
    return n;
  };
  feed("ab\177cd\r");
  check(console.read(destination, sizeof(input)) == 4, "edited line length");
  check(std::string(input, 4) == "acd\n", "backspace and CR translation");
  check(serial.output == "ab\b \bcd\n", "editing echo");
  feed("x\025y\n");
  check(console.read(destination, 1) == 1 && input[0] == 'y', "partial line");
  check(console.read(destination, 10) == 1 && input[0] == '\n', "remaining line");
  feed("z\004");
  check(console.read(destination, 8) == 1 && input[0] == 'z', "EOF after data");
  feed("next\n\004");
  check(console.read(destination, 8) == 5, "EOF does not create extra empty read");
  check(console.read(destination, 8) == 0, "separate empty EOF record");
  feed("q\004");
  check(console.read(destination, 1) == 1, "EOF at exact read limit");
  feed("r\n");
  check(console.read(destination, 8) == 2, "exact limit consumed EOF");
  for (int round = 0; round < 3; ++round)
  {
    feed(std::string(140, 'a' + round));
    check(queued() == 0, "overflow does not commit partial canonical line");
    feed("\n");
    check(console.read(destination, 256) == 128 && input[127] == '\n',
          "delimiter reserved at ring limit");
    for (int i = 0; i < 127; ++i)
    {
      check(input[i] == 'a' + round, "ring wrap data");
    }
  }
  settings.c_cc[VERASE] = '#';
  settings.c_cc[VEOL] = ';';
  set();
  feed("ab#c;");
  check(console.read(destination, 256) == 3 && std::string(input, 3) == "ac;",
        "configurable erase and delimiter");
  feed("word\ttail\027X\n");
  check(console.read(destination, 256) == 7 && std::string(input, 7) == "word\tX\n",
        "word erase");
  feed("\026\004\n");
  check(console.read(destination, 256) == 2 && input[0] == 4, "literal EOF");
  feed("unfinished");
  settings = defaults;
  settings.c_lflag = 0;
  settings.c_iflag = 0;
  settings.c_cc[VMIN] = 0;
  set();
  check(console.read(destination, 256) == 10, "mode switch publishes pending input");
  check(console.read(destination, 256) == 0, "MIN=0 TIME=0 immediate");
  serial.output.clear();
  const char raw[] = {0, 4, '\r', '\n', 127, 16};
  for (auto c : raw)
  {
    console.receive(c);
  }
  check(queued() == 6, "raw binary count");
  check(console.read(destination, 256) == 6 && std::memcmp(input, raw, 6) == 0,
        "raw preserves NUL, EOF, CR, newline and controls");
  check(serial.output.empty(), "raw disables echo");
  settings.c_iflag = IGNCR | INLCR | ISTRIP;
  set();
  console.receive('\r');
  console.receive('\n');
  console.receive(0xe1);
  check(console.read(destination, 256) == 2 && input[0] == '\r' && input[1] == 'a',
        "input ignore, NL mapping and strip");
  settings.c_iflag = 0;
  settings.c_cc[VMIN] = 3;
  set();
  int sleeps = 0;
  on_sleep = [&]
  {
    ++sleeps;
    feed(sleeps == 1 ? "a" : "bc");
  };
  check(console.read(destination, 256) == 3 && sleeps == 2, "MIN blocks for bytes");
  settings.c_cc[VMIN] = 200;
  set();
  sleeps = 0;
  on_sleep = [&]
  {
    ++sleeps;
    feed(std::string(100, 'v'));
  };
  check(console.read(destination, 256) == 200 && sleeps == 2,
        "MIN larger than ring capacity makes progress");
  settings.c_cc[VMIN] = 3;
  settings.c_cc[VTIME] = 2;
  set();
  sleeps = 0;
  on_sleep = [&]
  {
    ++sleeps;
    milliseconds += 100;
    if (sleeps == 1 || sleeps == 2)
    {
      feed("a");
    }
    console.timer_tick();
  };
  check(console.read(destination, 256) == 2 && sleeps == 4,
        "interbyte timer waits for first byte and restarts");
  settings.c_cc[VMIN] = 0;
  set();
  sleeps = 0;
  on_sleep = [&]
  {
    ++sleeps;
    milliseconds += 100;
    console.timer_tick();
  };
  check(console.read(destination, 256) == 0 && sleeps == 2, "read timer expires");
  on_sleep = [&] { feed("x"); };
  check(console.read(destination, 256) == 1 && input[0] == 'x',
        "MIN=0 TIME>0 returns on first byte");
  settings.c_cc[VMIN] = 2;
  settings.c_cc[VTIME] = 0;
  set();
  on_sleep = [&] { feed("y"); };
  check(console.read(destination, 1) == 1 && input[0] == 'y', "MIN clamps to count");
  on_sleep = [&] { killed = true; };
  check(console.read(destination, 256) == -1, "killed waiting reader");
  killed = false;
  on_sleep = nullptr;
  settings.c_cc[VMIN] = 0;
  settings.c_oflag = OPOST | ONLCR;
  set();
  char output[] = "a\nb";
  check(console.write(buffer(output), 3) == 3 && serial.output == "a\r\nb",
        "ONLCR output counts source bytes");
  settings.c_oflag = ONLCR;
  set();
  serial.output.clear();
  check(console.write(buffer(output), 3) == 3 && serial.output == output,
        "OPOST disables output translation");
  fail_copy = true;
  check(console.write(buffer(output), 3) == -1, "write copy failure");
  check(console.ioctl(TCSETS, buffer(&defaults)) == -1, "ioctl copy failure");
  fail_copy = false;
  feed("z");
  fail_copy = true;
  check(console.read(destination, 1) == -1, "read copy failure");
  fail_copy = false;
  check(console.read(destination, 1) == 1 && input[0] == 'z', "fault retains byte");
  settings.c_lflag |= 0x80000000U;
  check(console.ioctl(TCSETS, buffer(&settings)) == -1, "unsupported flags rejected");
  settings = defaults;
  check(console.ioctl(TCSETSF, buffer(&settings)) == 0 && serial.drains == 1,
        "flush setter drains");
  feed("secret");
  check(console.ioctl(TCFLSH, {AddressSpace::Kernel, TCIFLUSH}) == 0,
        "flush pending canonical input");
  feed("ok\n");
  check(console.read(destination, 256) == 3 && std::string(input, 3) == "ok\n",
        "flush discards pending line");
  winsize size{40, 100, 0, 0}, actual{};
  check(console.ioctl(TIOCSWINSZ, buffer(&size)) == 0 &&
            console.ioctl(TIOCGWINSZ, buffer(&actual)) == 0 && actual.ws_row == 40 &&
            actual.ws_col == 100,
        "window size round trip");
  check(console.ioctl(0xdead, destination) == -1, "unknown ioctl");
  serial.output.clear();
  feed("discard");
  console.receive(3);
  check(interrupts == 1 && queued() == 0 && serial.output == "discard^C\n",
        "VINTR flushes input and posts foreground interrupt");
  feed("\026\003\n");
  check(console.read(destination, 256) == 2 && input[0] == 3 && interrupts == 1,
        "literal-next quotes VINTR");
  settings.c_lflag &= ~ISIG;
  set();
  feed("\003\n");
  check(console.read(destination, 256) == 2 && input[0] == 3 && interrupts == 1,
        "disabled ISIG preserves interrupt byte");
  settings.c_lflag |= ISIG;
  settings.c_cc[VINTR] = 24;
  set();
  console.receive(24);
  check(interrupts == 2 && queued() == 0, "configurable VINTR");
  console.receive(26);
  check(interrupts == 3 && queued() == 0, "VSUSP posts stop action");
}

class FakeController final : public InterruptController
{
public:

  unsigned next = 0;
  unsigned completed = 0;
  unsigned priorities = 0;
  unsigned enabled = 0;
  int last_hart = -1;
  int *phase = nullptr;

  const char *name() const override
  {
    return "test controller";
  }

  void initialize() override
  {
  }

  void set_priority(unsigned irq, unsigned value) override
  {
    check(value == 1, "enabled source priority");
    priorities |= 1U << irq;
  }

  void initialize_hart(int hart) override
  {
    last_hart = hart;
  }

  void enable(int hart, unsigned irq) override
  {
    check(hart == last_hart, "hart configured before enable");
    enabled |= 1U << irq;
  }

  unsigned claim(int hart) override
  {
    check(hart == last_hart, "claim hart");
    return next;
  }

  void complete(int hart, unsigned irq) override
  {
    check(hart == last_hart, "completion hart");
    if (phase)
    {
      check(*phase == 1, "completion follows handler");
      *phase = 2;
    }
    completed = irq;
  }
};

class FakeInterrupt final : public InterruptDevice
{
public:

  unsigned source;
  int &phase;

  FakeInterrupt(unsigned irq, int &phase) : source(irq), phase(phase)
  {
  }

  const char *name() const override
  {
    return "test IRQ source";
  }

  void initialize() override
  {
  }

  unsigned irq() const override
  {
    return source;
  }

  void handle_interrupt() override
  {
    check(phase == 0, "single dispatch");
    phase = 1;
  }
};

void interrupt_contract()
{
  FakeController controller;
  int phase = 0;
  FakeInterrupt device(10, phase);
  InterruptDevice *sources[] = {&device};
  InterruptRouter router(controller, sources, 1);
  router.initialize();
  router.initialize_hart(2);
  check(controller.priorities == (1U << 10) && controller.enabled == (1U << 10),
        "router configures its sources");
  controller.next = 10;
  controller.phase = &phase;
  check(router.dispatch(2) == 0 && phase == 2, "known interrupt dispatch");
  controller.phase = nullptr;
  controller.next = 31;
  check(router.dispatch(2) == 31 && controller.completed == 31,
        "unknown IRQ completed");
  controller.completed = 0;
  controller.next = 0;
  check(router.dispatch(2) == 0 && controller.completed == 0,
        "spurious IRQ not completed");

  std::vector<unsigned> registers(0x208000 / 4, ~0U);
  Plic plic(reinterpret_cast<unsigned long>(registers.data()));
  plic.initialize();
  plic.set_priority(10, 1);
  plic.initialize_hart(1);
  plic.enable(1, 10);
  check(registers[10] == 1, "PLIC source priority");
  check(registers[(0x2080 + 0x100) / 4] == (1U << 10), "PLIC hart enable mask");
  check(registers[(0x2080 + 0x100) / 4 + 1] == 0, "PLIC clears all enable words");
  auto claim = (0x201004 + 0x2000) / 4;
  registers[claim] = 10;
  check(plic.claim(1) == 10, "PLIC claim register");
  plic.complete(1, 10);
  check(registers[claim] == 10, "PLIC completion register");
}

void uart_contract()
{
  alignas(4) unsigned char registers[8]{};
  Uart16550 uart(reinterpret_cast<unsigned long>(registers), 10);
  FakeSerial receiver_storage;
  NttyDiscipline receiver_discipline;
  Tty receiver(receiver_storage, receiver_discipline);
  uart.attach(receiver);
  uart.initialize();
  check(registers[1] == 3 && registers[3] == 3 && registers[2] == 7,
        "UART interrupt, line and FIFO configuration");
  registers[5] = 1U << 5;
  uart.put_sync('p');
  check(registers[0] == 'p', "UART polling output");
  registers[5] |= 1U << 6;
  uart.drain();
  uart.write("abc", 3);
  check(registers[0] == 'c', "UART process output");
}

VirtioBlock *active_disk = nullptr;
unsigned *active_registers = nullptr;
unsigned short device_used = 0;
unsigned transfer_count = 0;
unsigned long expected_sector = 0;
bool expected_write = false;

unsigned long address_register(unsigned low)
{
  return static_cast<unsigned long>(active_registers[low / 4]) |
         (static_cast<unsigned long>(active_registers[(low + 4) / 4]) << 32);
}

void complete_transfer()
{
  auto *desc =
      reinterpret_cast<virtq_desc *>(address_register(VIRTIO_MMIO_QUEUE_DESC_LOW));
  auto *avail =
      reinterpret_cast<virtq_avail *>(address_register(VIRTIO_MMIO_DRIVER_DESC_LOW));
  auto *used =
      reinterpret_cast<virtq_used *>(address_register(VIRTIO_MMIO_DEVICE_DESC_LOW));
  const auto head = avail->ring[(avail->idx - 1) % QueueSize];
  const auto middle = desc[head].next;
  const auto tail = desc[middle].next;
  auto *request = reinterpret_cast<virtio_blk_req *>(desc[head].addr);
  check(request->sector == expected_sector,
        "sector is not coupled to filesystem block size");
  check(request->type == (expected_write ? VIRTIO_BLK_T_OUT : VIRTIO_BLK_T_IN),
        "block direction");
  check(desc[head].len == sizeof(virtio_blk_req) && desc[tail].len == 1,
        "request chain lengths");
  check(bool(desc[middle].flags & VRING_DESC_F_WRITE) == !expected_write,
        "DMA data direction");
  check(desc[middle].len == 1024, "requested byte count");
  auto *data = reinterpret_cast<unsigned char *>(desc[middle].addr);
  if (expected_write)
  {
    check(data[0] == 0x5a, "device sees caller write data");
  }
  else
  {
    std::memset(data, 0x5a, desc[middle].len);
  }
  *reinterpret_cast<unsigned char *>(desc[tail].addr) = 0;
  used->ring[device_used % QueueSize].id = head;
  ++device_used;
  used->idx = device_used;
  active_registers[VIRTIO_MMIO_INTERRUPT_STATUS / 4] = 1;
  active_disk->handle_interrupt();
  ++transfer_count;
}

void block_contract()
{
  unsigned registers[0x100 / 4]{};
  registers[VIRTIO_MMIO_MAGIC_VALUE / 4] = 0x74726976;
  registers[VIRTIO_MMIO_VERSION / 4] = 2;
  registers[VIRTIO_MMIO_DEVICE_ID / 4] = 2;
  registers[VIRTIO_MMIO_VENDOR_ID / 4] = 0x554d4551;
  registers[VIRTIO_MMIO_QUEUE_NUM_MAX / 4] = QueueSize;
  VirtioBlock disk(reinterpret_cast<unsigned long>(registers), 1);
  disk.initialize();
  check(registers[VIRTIO_MMIO_QUEUE_READY / 4] == 1, "VirtIO queue ready");
  check((registers[VIRTIO_MMIO_STATUS / 4] & VIRTIO_CONFIG_S_DRIVER_OK) != 0,
        "VirtIO driver ready");
  active_registers = registers;
  active_disk = &disk;
  on_sleep = complete_transfer;
  unsigned char data[1024]{};
  for (unsigned i = 0; i < 24; ++i)
  {
    expected_sector = 7 + i;
    expected_write = i % 2;
    BlockDevice &device = disk;
    device.transfer(expected_write ? BlockOperation::Write : BlockOperation::Read,
                    expected_sector, data, sizeof(data));
    check(data[0] == 0x5a && data[1023] == 0x5a, "DMA completes before return");
  }
  check(transfer_count == 24, "queue wraps and descriptors are recycled");
  check(registers[VIRTIO_MMIO_INTERRUPT_ACK / 4] == 1, "VirtIO interrupt acknowledged");
  expect_panic([&] { disk.transfer(BlockOperation::Read, 0, data, 513); });
  auto *used =
      reinterpret_cast<virtq_used *>(address_register(VIRTIO_MMIO_DEVICE_DESC_LOW));
  used->ring[used->idx % QueueSize].id = QueueSize;
  ++used->idx;
  expect_panic([&] { disk.handle_interrupt(); });
  on_sleep = nullptr;
  for (auto *page : dma_pages)
  {
    std::free(page);
  }
  dma_pages.clear();
}
} // namespace test

namespace detail
{
unsigned long terminal_milliseconds()
{
  return test::milliseconds;
}

void prepare_interruptible_sleep(void *)
{
}

void prepare_sleep(void *)
{
}

void sleep_process()
{
  test::check(test::on_sleep != nullptr, "unexpected blocking path");
  test::on_sleep();
}

void wake_processes(void *)
{
  ++test::wakeups;
}

int claim_terminal(void *)
{
  return 0;
}

int terminal_foreground(void *)
{
  return 1;
}

int set_terminal_foreground(void *, int group)
{
  return group > 0 ? 0 : -1;
}

bool may_read_terminal(void *)
{
  return true;
}

void post_terminal_interrupt(void *, bool)
{
  ++test::interrupts;
}

bool process_killed()
{
  return test::killed;
}

void detach_terminal(void *) {}

void dump_processes()
{
  ++test::dumps;
}

int copy_in(void *destination, bool user, unsigned long source, unsigned count)
{
  test::copied_user = user;
  if (test::fail_copy)
  {
    return -1;
  }
  std::memcpy(destination, reinterpret_cast<const void *>(source), count);
  return 0;
}

int copy_out(bool user, unsigned long destination, void *source, unsigned count)
{
  test::copied_user = user;
  if (test::fail_copy)
  {
    return -1;
  }
  std::memcpy(reinterpret_cast<void *>(destination), source, count);
  return 0;
}

void push_interrupts_off()
{
}

void pop_interrupts_off()
{
}

bool panicking()
{
  return false;
}

bool panicked()
{
  return false;
}

void io_barrier()
{
  __atomic_thread_fence(__ATOMIC_SEQ_CST);
}

void *allocate_dma_page()
{
  void *page = std::aligned_alloc(4096, 4096);
  test::check(page != nullptr, "DMA page allocation");
  test::dma_pages.push_back(page);
  return page;
}

void zero_page(void *page)
{
  std::memset(page, 0, 4096);
}

[[noreturn]] void fail(const char *)
{
  std::_Exit(99);
}
} // namespace detail
} // namespace xv::drivers

extern "C" int run_driver_contracts()
{
  xv::drivers::test::console_contract();
  xv::drivers::test::interrupt_contract();
  xv::drivers::test::uart_contract();
  xv::drivers::test::block_contract();
  std::puts("driver contracts: passed");
  return 0;
}
