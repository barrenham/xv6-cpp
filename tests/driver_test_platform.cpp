module;

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <type_traits>
#include <sys/wait.h>
#include <unistd.h>

module xv.drivers;

import :platform;
import :console;
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

void (*on_sleep)() = nullptr;
bool killed = false;
bool fail_copy = false;
bool copied_user = false;
unsigned wakeups = 0;
unsigned dumps = 0;
std::vector<void *> dma_pages;

class FakeSerial final : public SerialPort
{
public:

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
  LineConsole console(serial);
  console.initialize();
  char input[256]{};
  auto destination =
      IoBuffer{AddressSpace::Kernel, reinterpret_cast<unsigned long>(input)};
  for (char c : std::string("ab\bcd\r"))
  {
    console.receive(c);
  }
  check(console.read(destination, sizeof(input)) == 4, "edited line length");
  check(std::string(input, 4) == "acd\n", "backspace and CR translation");
  check(serial.output == "ab\b \bcd\n", "synchronous editing echo");

  console.receive('x');
  console.receive(21); // Ctrl-U removes the uncommitted line.
  console.receive('y');
  console.receive('\n');
  check(console.read(destination, 1) == 1 && input[0] == 'y', "partial line read");
  check(console.read(destination, 10) == 1 && input[0] == '\n', "remaining line read");

  console.receive('z');
  console.receive(4);
  check(console.read(destination, 8) == 1 && input[0] == 'z', "EOF after data");
  check(console.read(destination, 8) == 0, "EOF preserved for next reader");

  // A full line buffer is delivered without a newline; wrap multiple times.
  for (int round = 0; round < 3; ++round)
  {
    for (int i = 0; i < 128; ++i)
    {
      console.receive('a' + round);
    }
    console.receive('!'); // Full buffer discards additional input.
    check(console.read(destination, 128) == 128, "full ring delivery");
    for (int i = 0; i < 128; ++i)
    {
      check(input[i] == 'a' + round, "ring wrap data");
    }
  }

  const std::string message(79, 'w');
  serial.output.clear();
  auto source =
      IoBuffer{AddressSpace::User, reinterpret_cast<unsigned long>(message.data())};
  check(console.write(source, message.size()) == 79, "batched console output");
  check(serial.output == message && copied_user, "user copy routing");
  fail_copy = true;
  check(console.write(source, 4) == 0, "failed input copy stops write");
  fail_copy = false;
  killed = true;
  check(console.read(destination, 4) == -1, "killed waiting reader");
  killed = false;
  console.receive(16);
  check(dumps == 1 && wakeups > 0, "process dump and reader wakeups");
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
  LineConsole receiver(receiver_storage);
  uart.attach(receiver);
  uart.initialize();
  check(registers[1] == 3 && registers[3] == 3 && registers[2] == 7,
        "UART interrupt, line and FIFO configuration");
  registers[5] = 1U << 5;
  uart.put_sync('p');
  check(registers[0] == 'p', "UART polling output");
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

bool process_killed()
{
  return test::killed;
}

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
