module;
#include "kernel/support/placement_new.hpp"
#include "kernel/support/flow.h"

module xv.fs;
import :pipe;
import :platform;

namespace xv::fs
{
namespace
{
namespace flow = xv::flow;

struct PipeFactoryFlow
{
  struct Context
  {
    File **read_end, **write_end;
    File *reader = nullptr, *writer = nullptr;
    void *storage = nullptr;
    bool result = false;
  };

  static void allocate_files(Context &c) noexcept
  {
    c.reader = files().allocate();
    c.writer = files().allocate();
  }

  static bool both(Context &c) noexcept
  {
    return c.reader && c.writer;
  }

  static bool allocate_page(Context &c) noexcept
  {
    c.storage = detail::allocate_page();
    return c.storage != nullptr;
  }

  static void construct(Context &c) noexcept
  {
    auto *pipe = new (c.storage) Pipe;
    pipe->initialize();
    files().bind_pipe(*c.reader, *pipe, false);
    files().bind_pipe(*c.writer, *pipe, true);
  }

  static void publish(Context &c) noexcept
  {
    *c.read_end = c.reader;
    *c.write_end = c.writer;
    c.result = true;
  }

  static bool has_reader(Context &c) noexcept
  {
    return c.reader != nullptr;
  }

  static bool has_writer(Context &c) noexcept
  {
    return c.writer != nullptr;
  }

  static void release_reader(Context &c) noexcept
  {
    files().release(c.reader);
  }

  static void release_writer(Context &c) noexcept
  {
    files().release(c.writer);
  }
  struct Files;
  struct Storage;
  struct Construct;
  struct Publish;
  struct Reader;
  struct ReleaseReader;
  struct Writer;
  struct ReleaseWriter;
  using Graph = flow::graph<
      Files,
      flow::types<
          flow::node<Files, flow::action<&allocate_files>,
                     flow::branch<flow::condition<&both>, flow::go<Storage>,
                                  flow::go<Reader>>>,
          flow::node<Storage, flow::noop,
                     flow::branch<flow::condition<&allocate_page>, flow::go<Construct>,
                                  flow::go<Reader>>>,
          flow::node<Construct, flow::action<&construct>, flow::go<Publish>>,
          flow::node<Publish, flow::action<&publish>, flow::stop>,
          flow::node<Reader, flow::noop,
                     flow::branch<flow::condition<&has_reader>, flow::go<ReleaseReader>,
                                  flow::go<Writer>>>,
          flow::node<ReleaseReader, flow::action<&release_reader>, flow::go<Writer>>,
          flow::node<Writer, flow::noop,
                     flow::branch<flow::condition<&has_writer>, flow::go<ReleaseWriter>,
                                  flow::stop>>,
          flow::node<ReleaseWriter, flow::action<&release_writer>, flow::stop>>>;
};
} // namespace

bool create_pipe(File **read_end, File **write_end)
{
  *read_end = nullptr;
  *write_end = nullptr;
  PipeFactoryFlow::Context context{read_end, write_end};
  (void)PipeFactoryFlow::Graph::run<0>(context);
  return context.result;
}
} // namespace xv::fs
