#pragma once
#include "state.h"

namespace fbterm_app
{
// A renderer is the only process allowed to draw after start(). The I/O
// process sends at most one cell snapshot and continues servicing the PTY.
class Presenter
{
public:

  int writer = -1, reader = -1, pid = -1;
  bool busy = false;
  uint64 completed_cost = 0, completed_rows = 0;

  bool start(int framebuffer, int master);

  bool enqueue();

  bool writing() const;

  bool progress();

  bool finish(bool dirty);

  void close_worker();

private:

  int framebuffer_ = -1;
  bool synchronous_ = false;
  unsigned wire_size_ = 0;

  struct Reply
  {
    Diagnostics diagnostics;
    FrameSample sample;
    uint64 raster_rows, reused_rows, move_ticks, sample_count, cost;
    bool okay;
  } reply_{};

  unsigned sent_ = 0, received_ = 0;
};

extern Presenter presenter;
} // namespace fbterm_app
