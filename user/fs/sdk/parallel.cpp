module;
#include "user/runtime/user.h"
#include "kernel/fs/poll.h"
#include "kernel/process/wait/wait.h"
#include "kernel/support/placement_new.hpp"
#include "kernel/fs/userfs/protocol.h"
module xv.userfs;

namespace userfs
{
namespace
{
void
child_event(int) {
}

struct Workers
{
  int pids[xv::userfs::TransportLanes]{};
  unsigned limit = 0;

  bool
  reap(bool wait_all) {
    bool good = true;
    for (auto &pid : pids)
    {
      if (!pid)
      {
        continue;
      }
      int status = -1;
      int result;
      do
      {
        result = waitpid(pid, &status, wait_all ? 0 : WNOHANG);
      } while (result < 0 && errno == EINTR);
      if (result == pid)
      {
        pid = 0;
        good &= status == 0;
      }
      else if (result < 0)
      {
        return false;
      }
    }
    return good;
  }

  bool
  active() const {
    for (int pid : pids)
    {
      if (pid)
      {
        return true;
      }
    }
    return false;
  }

  void
  stop() {
    for (int pid : pids)
    {
      if (pid)
      {
        kill(pid);
      }
    }
    reap(true);
  }

  ~Workers() {
    stop();
  }
};
} // namespace

int
serve_parallel(int descriptor, Backend &backend) {
  // Each request has a separate process stack/heap and a distinct shared bulk
  // slice. The parent alone mutates backend state. It waits for all snapshot
  // readers before mutations, Forget, maintenance or metadata transactions.
  struct State
  {
    xv::userfs::Message message;
    int channels[xv::userfs::TransportLanes]{-1, -1, -1};
    unsigned lanes = 0, payload = 0, bytes = 0;
    void *mapping = nullptr;
    struct sigaction previous{};
    bool signal_installed = false;
    Workers workers;

    ~State() {
      workers.stop();
      for (unsigned i = 0; i < lanes; ++i)
      {
        if (channels[i] >= 0)
        {
          close(channels[i]);
        }
      }
      if (mapping)
      {
        munmap(mapping, bytes);
      }
      if (signal_installed)
      {
        sigaction(SIGCHLD, &previous, nullptr);
      }
    }
  };

  auto *memory = malloc(sizeof(State));
  if (!memory)
  {
    return -1;
  }
  auto *state = new (memory) State{};

  struct Cleanup
  {
    State *state;

    ~Cleanup() {
      state->~State();
      free(state);
    }
  } cleanup{state};

  state->channels[0] = backend.transport_;
  state->lanes = backend.transport_lanes_;
  for (unsigned i = 1; i < state->lanes; ++i)
  {
    state->channels[i] = backend.extra_transports_[i - 1];
  }
  state->payload = backend.bulk_bytes_;
  state->mapping = backend.bulk_;
  state->bytes = backend.bulk_mapping_bytes_;
  state->workers.limit = backend.parallel_read_workers();
  if (state->workers.limit > state->lanes - 1)
  {
    state->workers.limit = state->lanes - 1;
  }
  backend.transport_ = -1;
  backend.bulk_ = nullptr;
  struct sigaction action{};
  action.sa_handler = child_event;
  if (sigaction(SIGCHLD, &action, &state->previous) < 0)
  {
    return -1;
  }
  state->signal_installed = true;
  unsigned cursor = 0;
  for (;;)
  {
    if (!state->workers.reap(false))
    {
      return -1;
    }
    if (backend.maintenance_pending())
    {
      if (!state->workers.reap(true) || !backend.maintain())
      {
        return -1;
      }
    }
    pollfd watched[xv::userfs::TransportLanes]{};
    for (unsigned i = 0; i < state->lanes; ++i)
    {
      watched[i] = {state->workers.pids[i] ? -1 : state->channels[i], POLLIN, 0};
    }
    int delay = state->workers.active() ? 10 : -1;
    if (backend.maintenance_pending())
    {
      delay = backend.maintenance_delay_ms();
    }
    int ready = poll(watched, state->lanes, delay);
    if (ready < 0)
    {
      if (errno == EINTR)
      {
        continue;
      }
      return -1;
    }
    if (!ready)
    {
      continue;
    }
    unsigned lane = cursor;
    for (unsigned n = 0; n < state->lanes; ++n)
    {
      lane = (cursor + n) % state->lanes;
      if (watched[lane].revents)
      {
        break;
      }
    }
    cursor = (lane + 1) % state->lanes;
    auto &message = state->message;
    int count = read(state->channels[lane], &message, sizeof(message));
    if (!count)
    {
      return 0;
    }
    if (count != sizeof(message))
    {
      return -1;
    }
    auto *data = static_cast<char *>(state->mapping) + lane * state->payload;
    bool parallel = lane && lane <= state->workers.limit &&
                    message.operation == xv::userfs::Operation::Read;
    if (parallel)
    {
      int pid = fork();
      if (!pid)
      {
        dispatch(message, backend, data, state->payload);
        int sent = write(state->channels[lane], &message, sizeof(message));
        // exit() deliberately bypasses parent-owned RAII cleanup.
        exit(sent == sizeof(message) ? 0 : 1);
      }
      if (pid < 0)
      {
        return -1;
      }
      state->workers.pids[lane] = pid;
    }
    else
    {
      if (!state->workers.reap(true))
      {
        return -1;
      }
      dispatch(message, backend, data, state->payload);
      if (write(state->channels[lane], &message, sizeof(message)) != sizeof(message))
      {
        return -1;
      }
    }
  }
}
} // namespace userfs
