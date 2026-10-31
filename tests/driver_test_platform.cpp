module;

#include <cstdio>
#include <functional>
#include "kernel/tty/termios.h"
#include "kernel/fs/io_status.h"
#include "kernel/fs/poll.h"
#include "kernel/fs/io_status.h"
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <type_traits>
#include <sys/wait.h>
#include <sys/mman.h>
#include <unistd.h>

module xv.drivers;

import :platform;
import :tty;
import :controlling;
import :n_tty;
import :uart;
import :rtc;
import :entropy;
import :plic;
import :virtio;
import :protocol;
import :e1000;
import :pci;

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
bool fail_dma = false;
int dma_budget=-1;
unsigned hangups = 0, signal_services = 0;
Tty *controlling_tty=nullptr;

class FakeSerial final : public SerialPort
{
public:

  unsigned drains = 0;

  void drain() override
  {
    ++drains;
  }

  std::string output;
  std::function<void()> on_transmit;
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

  int transmit(const char *data, int count, bool = false) override
  {
    if (on_transmit) on_transmit();
    write(data, count);
    return count;
  }

  void put_sync(int character) override
  {
    output.push_back(character);
  }
};

void rtc_contract()
{
  alignas(8) unsigned registers[2]={0xf1234567U,0x12345678U};
  GoldfishRtc rtc(reinterpret_cast<unsigned long>(registers));rtc.initialize();
  check(rtc.nanoseconds()==0x12345678f1234567UL,"RTC combines all 64 timestamp bits");
  check(rtc.seconds()==0x12345678f1234567UL/1000000000UL,"RTC converts nanoseconds to epoch seconds");
  registers[0]=1;registers[1]=0;check(rtc.nanoseconds()==1,"RTC reads fresh device values");
}

void controlling_contract()
{
  FakeSerial serial;NttyDiscipline discipline;Tty tty(serial,discipline);tty.initialize();
  ControllingTerminal device;
  check(!device.open(false),"controlling tty requires a session binding");
  controlling_tty=&tty;auto *session=device.open(false);
  check(session==&tty,"console controlling tty resolves actual session");
  char value='Q';check(session->write({AddressSpace::Kernel,reinterpret_cast<unsigned long>(&value)},1)==1 && serial.output=="Q","controlling tty forwards to its open session");
  session->close();controlling_tty=nullptr;
}

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
  settings.c_iflag |= IGNCR;
  set();
  feed("\026\r\004\n");
  check(console.read(destination, 256) == 2 && input[0] == 4 && input[1] == '\n',
        "ignored CR preserves the pending literal-next quote");
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
  feed("m");
  sleeps = 0;
  on_sleep = [&]
  {
    check(++sleeps == 1, "configuration change must complete a partial read");
    settings.c_cc[VMIN] = 200;
    set();
  };
  check(console.read(destination, 256) == 1 && input[0] == 'm' && sleeps == 1,
        "generation change returns consumed bytes before adopting the new minimum");
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
  IoBuffer nonblocking = destination;
  nonblocking.nonblocking = true;
  check(console.read(nonblocking, sizeof(input)) == -EAGAIN,
        "empty nonblocking TTY does not sleep");
  on_sleep = [&]
  {
    check(console.read(nonblocking, sizeof(input)) == -EAGAIN,
          "nonblocking TTY reader never waits for another reader lock");
    feed("ready\n");
  };
  check(console.read(destination, sizeof(input)) == 6, "blocking reader survives contention");
  serial.on_transmit = [&]
  {
    check(console.write(nonblocking, 1) == -EAGAIN,
          "nonblocking TTY writer never waits for another writer lock");
  };
  check(console.write(destination, 1) == 1, "blocking writer survives contention");
  serial.on_transmit = nullptr;
  char chunks[65];
  std::memset(chunks, 'x', sizeof(chunks));
  serial.output.clear();
  serial.on_transmit = [&] { fail_copy = true; };
  check(console.write(buffer(chunks), sizeof(chunks)) == 32 && serial.output.size() == 32,
        "later source-copy failure preserves the completed source-byte count");
  fail_copy = false;
  serial.on_transmit = nullptr;
  on_sleep = nullptr;
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


void pty_contract()
{
  initialize_ptys();
  auto baseline = dma_pages.size();
  auto buffer = [](void *p, bool nonblocking = false)
  { return IoBuffer{AddressSpace::Kernel, reinterpret_cast<unsigned long>(p), nonblocking}; };
  CharacterDevice *master = nullptr, *slave = nullptr;
  fail_dma = true;
  check(!create_pty(&master, &slave) && !master && !slave && dma_pages.size() == baseline,
        "PTY allocation failure publishes no endpoints");
  fail_dma = false;
  check(create_pty(&master, &slave) && master && slave, "create actual PTY endpoints");
  termios raw{};
  check(slave->ioctl(TCGETS, buffer(&raw)) == 0, "PTY termios snapshot");
  raw.c_iflag = 0; raw.c_lflag = 0; raw.c_oflag = OPOST | ONLCR;
  raw.c_cc[VMIN] = 0; raw.c_cc[VTIME] = 0;
  check(slave->ioctl(TCSETS, buffer(&raw)) == 0, "PTY raw input with CRLF output");
  char input[1100]{};
  char message[] = "abc";
  check(master->write(buffer(message), 3) == 3 && slave->read(buffer(input), 3) == 3 &&
        std::string(input, 3) == "abc", "master input reaches actual TTY");
  char newline[] = "q\n";
  check(slave->write(buffer(newline), 2) == 2 && master->read(buffer(input), 3) == 3 &&
        std::string(input, 3) == "q\r\n", "PTY output expands newline atomically");
  std::memset(input, 'x', 1023);
  check(slave->write(buffer(input), 1023) == 1023, "fill all but one output slot");
  check(slave->write(buffer(newline + 1, true), 1) == -EAGAIN,
        "nonblocking CRLF does not consume the last single slot");
  int available = -1;
  check(master->ioctl(FIONREAD, buffer(&available)) == 0 && available == 1023,
        "failed CRLF publishes neither half");
  check(master->read(buffer(input), 1023) == 1023, "drain full PTY output");
  char value = 'z';
  check(slave->write(buffer(&value), 1) == 1, "queue output for copy failure");
  fail_copy = true;
  check(master->read(buffer(input), 1) == -1, "PTY copy failure reported");
  fail_copy = false;
  check(master->read(buffer(input), 1) == 1 && input[0] == 'z', "PTY fault retains byte");
  std::memset(input, 'a', 128);
  check(master->write(buffer(input), 128) == 128 && master->write(buffer(&value, true), 1) == -EAGAIN,
        "full software input uses nonblocking backpressure");
  unsigned sleeps = 0;
  on_sleep = [&]
  { check(++sleeps == 1, "PTY input retry should wake once"); char byte; check(slave->read(buffer(&byte), 1) == 1, "consume input while writer sleeps"); };
  check(master->write(buffer(&value), 1) == 1 && sleeps == 1,
        "software input registers before unlocking and retries after wake");
  on_sleep = nullptr;
  check(slave->read(buffer(input), 128) == 128 && input[127] == 'z', "retry preserves input order");
  sleeps = 0;
  on_sleep = [&]
  { check(++sleeps == 1, "PTY output reader wakes once"); check(slave->write(buffer(&value), 1) == 1, "publish output before reader resumes"); };
  check(master->read(buffer(input), 1) == 1 && input[0] == 'z', "blocking master read resumes");
  on_sleep = nullptr;
  check(slave->write(buffer(&value), 1) == 1, "queue output before termios drain");
  sleeps = 0;
  on_sleep = [&]
  { check(++sleeps == 1, "PTY drain wakes once"); check(master->read(buffer(input), 1) == 1, "drain callback holds no output lock"); };
  check(slave->ioctl(TCSETSW, buffer(&raw)) == 0 && sleeps == 1, "termios drain releases pair lock while sleeping");
  on_sleep = nullptr;
  master->close();
  check(slave->read(buffer(input), 1) == 0 && slave->write(buffer(&value), 1) == -1,
        "master close lets slave observe EOF and output hangup");
  slave->close();
  check(dma_pages.size() == baseline && hangups == 1 && signal_services > 0,
        "anonymous endpoints release pair once after software signal service");

  master = ptmx().open(false);
  unsigned number = MaxNamedPtys;
  check(master && master->ioctl(TIOCGPTN, buffer(&number)) == 0, "create named master");
  unsigned uid = 0, gid = 0;
  check(named_pty_owner(number, uid, gid) && uid == 1000 && gid == 100 && !open_named_pty(number),
        "named ownership is captured before publication and slave starts locked");
  int unlocked = 0;
  check(master->ioctl(TIOCSPTLCK, buffer(&unlocked)) == 0 && retain_named_pty(number), "unlock and pin named node");
  slave = open_named_pty(number);
  check(slave, "open named slave");
  master->close();
  unsigned ids[3]{};
  check(!open_named_pty(number) && !retain_named_pty(number) && named_pty_ids(ids, 3) == 0 &&
        named_pty_owner(number, uid, gid), "unpublished pair survives only through existing references");
  slave->close();
  check(dma_pages.size() == baseline + 1, "node pin survives endpoint closes");
  release_named_pty(number);
  check(dma_pages.size() == baseline, "last node pin releases pair");

  CharacterDevice *masters[MaxNamedPtys]{};
  for (unsigned i = 0; i < MaxNamedPtys; ++i)
  {
    masters[i] = ptmx().open(false); number = MaxNamedPtys;
    check(masters[i] && masters[i]->ioctl(TIOCGPTN, buffer(&number)) == 0 && number == i,
          "named allocation chooses lowest unused ID");
  }
  check(named_pty_ids(ids, 3) == 3 && ids[0] == 0 && ids[1] == 1 && ids[2] == 2,
        "named enumeration is sorted and capacity bounded");
  check(!ptmx().open(false) && dma_pages.size() == baseline + MaxNamedPtys,
        "named ID exhaustion destroys only the unpublished candidate");
  for (auto *endpoint : masters) endpoint->close();
  check(dma_pages.size() == baseline, "all named endpoints reclaim their pages");
}


void pci_contract()
{
  constexpr unsigned long base = 0x30000000UL;
  constexpr unsigned long span = 32UL << 15;
  auto *arena = mmap(reinterpret_cast<void *>(base), span, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
  check(arena != MAP_FAILED, "map isolated PCI config fixture without replacement");
  auto config = [&](unsigned slot) { return reinterpret_cast<unsigned *>(base + (static_cast<unsigned long>(slot) << 15)); };
  for (unsigned slot : {1U, 5U, 29U}) config(slot)[0] = 0x100e8086;
  unsigned slots[4]{};
  check(detail::find_e1000(nullptr, 0) == 0 && detail::find_e1000(slots, 2) == 2 &&
        slots[0] == 1 && slots[1] == 5, "PCI discovery honors output capacity and slot order");
  check(detail::find_e1000(slots, 4) == 3 && slots[2] == 29, "PCI discovery scans all 32 slots");
  config(0)[1] = 0x12345678; config(0)[4] = 0x98765432;
  check(!detail::configure_e1000(0x40000000, 32) &&
        !detail::configure_e1000(0x40000000, 0) && config(0)[1] == 0x12345678 &&
        config(0)[4] == 0x98765432, "invalid PCI slot/vendor leaves command and BAR untouched");
  config(5)[1] = 0xabcd0403;
  check(detail::configure_e1000(0x40020000, 5) && config(5)[4] == 0x40020000 &&
        config(5)[1] == 7, "PCI configuration preserves low command bits and enables memory/bus master IRQ");
  struct Receiver final : EthernetReceiver
  { void receive_frame(void *, unsigned) override { check(false, "unready NIC must not deliver frames"); } } receiver;
  unsigned registers[0x6000 / 4]{};
  E1000 nic(reinterpret_cast<unsigned long>(registers), 0);
  nic.initialize();
  check(!nic.online() && !nic.carrier(), "NIC without receiver remains offline");
  nic.attach(receiver); nic.locate(0); nic.initialize();
  check(!nic.online() && registers[0] == 0, "PCI rejection stops NIC before reset programming");
  auto before = dma_pages.size();
  auto *page = detail::allocate_dma_page();
  check(!nic.transmit(page, 64) && dma_pages.size() == before + 1,
        "offline NIC rejects TX without taking page ownership");
  detail::free_dma_page(page);
  check(dma_pages.size() == before, "rejected TX remains caller-owned");
  check(munmap(arena, span) == 0, "unmap isolated PCI fixture");
}

class FakeController final : public InterruptController
{
public:

  unsigned next = 0;
  unsigned completed = 0;
  unsigned completions = 0;
  bool forbid_initialize = false;
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
    if (forbid_initialize) std::_Exit(98);
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
    ++completions;
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
  check(controller.completions == 2, "each nonzero claim completes exactly once");
  FakeController guarded;
  guarded.forbid_initialize = true;
  InterruptDevice *duplicates[] = {&device, &device};
  InterruptDevice *missing[] = {nullptr};
  expect_panic([&] { InterruptRouter invalid(guarded, duplicates, 2); invalid.initialize(); });
  expect_panic([&] { InterruptRouter invalid(guarded, missing, 1); invalid.initialize(); });


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
  registers[5] = 0;
  char fill[128]; std::memset(fill, 'q', sizeof(fill));
  check(uart.transmit(fill, 127, true) == 127, "UART queues without waiting for hardware");
  const char newline[] = "\r\n";
  check(uart.transmit(newline, 2, true) == -EAGAIN &&
        uart.transmit("z", 1, true) == 1 && uart.transmit("x", 1, true) == -EAGAIN &&
        !uart.writable(), "UART atomic transformed output and full EAGAIN");
  registers[5] = (1U << 5) | (1U << 6);
  uart.handle_interrupt();
  check(registers[0] == 'z' && uart.writable(), "UART TX interrupt drains queued bytes");
  uart.drain();

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
  auto *request = reinterpret_cast<virtio_blk_req *>(desc[head].addr);
  if (request->type == 4)
  {
    check(request->sector == 0 && desc[middle].len == 1 && (desc[middle].flags & VRING_DESC_F_WRITE) && !(desc[middle].flags & VRING_DESC_F_NEXT), "flush has only header and writable status");
    *reinterpret_cast<unsigned char *>(desc[middle].addr) = 0;
    used->ring[device_used % QueueSize].id = head; used->idx = ++device_used;
    active_registers[VIRTIO_MMIO_INTERRUPT_STATUS / 4] = 1;
    active_disk->handle_interrupt(); ++transfer_count; return;
  }
  const auto tail = desc[middle].next;
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

void entropy_contract()
{
  unsigned registers[0x108/4]{};
  VirtioEntropy missing(reinterpret_cast<unsigned long>(registers),3);missing.initialize();
  char bytes[600]{};IoBuffer kernel{AddressSpace::Kernel,reinterpret_cast<unsigned long>(bytes),true};
  check(missing.read(kernel,1)==-1 && missing.read(kernel,0)==0 && missing.poll()==POLLERR,"missing entropy never fabricates bytes or readiness");
  auto setup=[&]{std::memset(registers,0,sizeof(registers));
    registers[VIRTIO_MMIO_MAGIC_VALUE/4]=0x74726976;registers[VIRTIO_MMIO_VERSION/4]=2;
    registers[VIRTIO_MMIO_DEVICE_ID/4]=4;registers[VIRTIO_MMIO_DEVICE_FEATURES/4]=1;
    registers[VIRTIO_MMIO_QUEUE_NUM_MAX/4]=QueueSize;};
  unsigned baseline=dma_pages.size();
  for(int budget=0;budget<4;++budget)
  {
    setup();dma_budget=budget;VirtioEntropy failed(reinterpret_cast<unsigned long>(registers),3);failed.initialize();dma_budget=-1;
    check(dma_pages.size()==baseline && failed.poll()==POLLERR && (registers[VIRTIO_MMIO_STATUS/4]&128),"entropy initialization rolls back each DMA allocation failure");
  }
  setup();VirtioEntropy rng(reinterpret_cast<unsigned long>(registers),3);rng.initialize();
  auto address=[&](unsigned low){return registers[low/4]|(static_cast<unsigned long>(registers[(low+4)/4])<<32);};
  auto *desc=reinterpret_cast<virtq_desc *>(address(VIRTIO_MMIO_QUEUE_DESC_LOW));
  auto *avail=reinterpret_cast<virtq_avail *>(address(VIRTIO_MMIO_DRIVER_DESC_LOW));
  auto *used=reinterpret_cast<virtq_used *>(address(VIRTIO_MMIO_DEVICE_DESC_LOW));
  check(avail->idx==1 && desc[0].flags==VRING_DESC_F_WRITE && desc[0].len==4096,"entropy submits only writable buffers after modern feature negotiation");
  auto complete=[&](unsigned length,unsigned id=0,unsigned char fill=0x5a){
    std::memset(reinterpret_cast<void *>(desc[0].addr),fill,length<=4096 ? length:0);
    used->ring[used->idx%QueueSize]={id,length};++used->idx;
    registers[VIRTIO_MMIO_INTERRUPT_STATUS/4]=1;rng.handle_interrupt();};
  class Watch final:public xv::sync::PollTable
  {public:void *channel=nullptr;void watch(void *value) override {channel=value;}} watch;
  check(rng.poll(&watch)==0 && watch.channel && rng.read(kernel,1)==-EAGAIN,"entropy poll registration precedes nonblocking empty read");
  unsigned woke=wakeups;complete(17);
  check(wakeups>woke && rng.poll()==POLLIN,"entropy completion wakes readers and poll waiters");
  fail_copy=true;check(rng.read(kernel,17)==-1,"entropy copy failure");fail_copy=false;
  check(rng.read(kernel,600)==17 && bytes[0]==0x5a && bytes[16]==0x5a && avail->idx==2,"entropy honors short used length and preserves bytes after copy failure");
  expect_panic([&]{complete(0);});expect_panic([&]{complete(4097);});expect_panic([&]{complete(1,7);});
  kernel.nonblocking=false;on_sleep=[&]{complete(4096,0,0x33);};
  check(rng.read(kernel,600)==256 && bytes[255]==0x33,"blocking entropy read wakes and bounds interrupt-disabled copy");
  kernel.nonblocking=true;unsigned total=256;
  for(int result;(result=rng.read(kernel,sizeof(bytes)))>0;)total+=result;
  check(total==4096 && rng.read(kernel,1)==-EAGAIN,"entropy consumes completed buffer once and requests a refill");
  on_sleep=nullptr;
  for(unsigned i=0;i<65537;++i)
  {complete(1,0,static_cast<unsigned char>(i));check(rng.read(kernel,1)==1 && static_cast<unsigned char>(bytes[0])==static_cast<unsigned char>(i),"entropy ring indexes wrap without replay");}
  check(rng.write(kernel,1)==-1 && rng.write(kernel,0)==0,"host entropy source cannot be overwritten through its device");
  registers[VIRTIO_MMIO_STATUS/4]=0;
  while(dma_pages.size()>baseline)detail::free_dma_page(dma_pages.back());
}

void block_contract()
{
  unsigned registers[0x108 / 4]{};
  registers[VIRTIO_MMIO_MAGIC_VALUE / 4] = 0x74726976;
  registers[VIRTIO_MMIO_VERSION / 4] = 2;
  registers[VIRTIO_MMIO_DEVICE_ID / 4] = 2;
  registers[VIRTIO_MMIO_VENDOR_ID / 4] = 0x554d4551;
  registers[VIRTIO_MMIO_QUEUE_NUM_MAX / 4] = QueueSize;
  registers[VIRTIO_MMIO_DEVICE_FEATURES / 4] = 1U << VIRTIO_BLK_F_FLUSH;
  registers[VIRTIO_MMIO_CONFIG / 4] = 0x89abcdefU;
  registers[(VIRTIO_MMIO_CONFIG + 4) / 4] = 0x12345678U;
  VirtioBlock disk(reinterpret_cast<unsigned long>(registers), 1);
  disk.initialize();
  check(disk.capacity_sectors() == 0x1234567889abcdefUL,
        "VirtIO capacity preserves both halves of the 64-bit sector count");
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
  disk.flush();
  check(transfer_count == 25, "queue wraps and descriptors are recycled");
  for (unsigned i = 0; i < 16; ++i)
  {
    disk.flush();
    expected_sector = 100 + i; expected_write = false;
    disk.transfer(BlockOperation::Read, expected_sector, data, sizeof(data));
  }
  check(transfer_count == 57,
        "repeated flush and data requests recycle all three allocated slots");
  auto before_empty_irq = wakeups;
  disk.handle_interrupt();
  check(wakeups == before_empty_irq, "empty used queue does not repeat completion wakes");
  expect_panic([&]
  {
    on_sleep = [&]
    {
      auto *desc = reinterpret_cast<virtq_desc *>(address_register(VIRTIO_MMIO_QUEUE_DESC_LOW));
      auto *avail = reinterpret_cast<virtq_avail *>(address_register(VIRTIO_MMIO_DRIVER_DESC_LOW));
      auto *used = reinterpret_cast<virtq_used *>(address_register(VIRTIO_MMIO_DEVICE_DESC_LOW));
      auto head = avail->ring[(avail->idx - 1) % QueueSize];
      auto tail = desc[desc[head].next].next;
      *reinterpret_cast<unsigned char *>(desc[tail].addr) = 1;
      used->ring[device_used % QueueSize].id = head; used->idx = ++device_used;
      disk.handle_interrupt();
    };
    disk.transfer(BlockOperation::Read, expected_sector, data, sizeof(data));
  });

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
unsigned effective_uid() { return 1000; }
unsigned effective_gid() { return 100; }
void hangup_terminal(void *) { ++test::hangups; }
void service_terminal_signals() { ++test::signal_services; }
void free_terminal_page(void *page)
{
  auto &pages = test::dma_pages;
  for (auto it = pages.begin(); it != pages.end(); ++it)
    if (*it == page) { pages.erase(it); std::free(page); return; }
  test::check(false, "free terminal page exactly once");
}

void free_dma_page(void *page) { free_terminal_page(page); }

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

void *pin_controlling_terminal(void *(*pin)(void *))
{return test::controlling_tty ? pin(test::controlling_tty):nullptr;}

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
  if (test::fail_dma || test::dma_budget==0) return nullptr;
  if(test::dma_budget>0)--test::dma_budget;
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
  xv::drivers::test::rtc_contract();
  xv::drivers::test::controlling_contract();
  xv::drivers::test::console_contract();
  xv::drivers::test::pty_contract();
  xv::drivers::test::pci_contract();
  xv::drivers::test::interrupt_contract();
  xv::drivers::test::uart_contract();
  xv::drivers::test::block_contract();
  xv::drivers::test::entropy_contract();
  std::puts("driver contracts: passed");
  return 0;
}
