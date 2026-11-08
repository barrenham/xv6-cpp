#pragma once
#include "buffer.h"
#include "file.h"
#include "terminal.h"

namespace vim
{
class Editor final
{
public:

  int run(const char *path);

private:

  enum class Mode
  {
    Normal,
    Insert,
    Command,
    Search
  } mode_ = Mode::Normal;
  Buffer buffer_;
  File file_;
  Terminal terminal_;
  unsigned cursor_ = 0, top_ = 0, left_ = 0, input_size_ = 0;
  char input_[MAXPATH + 16]{}, search_[MAXPATH]{};
  const char *message_ = "i insert  :w save  :q quit  / search  :help";
  bool done_ = false;
  char pending_ = 0;
  void draw();
  void normal(int key);
  void insert(int key);
  void command(int key);
  void execute();
  void find();
  void normalize();
  bool snapshot();
  unsigned column(unsigned start, unsigned end) const;
};
} // namespace vim
