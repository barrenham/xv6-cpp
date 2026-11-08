#include "application.h"
#include "editor.h"
#include "kernel/support/placement_new.hpp"
#include "user/runtime/ui.h"

extern "C" int
app_main(int argc, char **argv) {
  if (argc == 2 && !strcmp(argv[1], "--help"))
  {
    ui::heading("vim [file] - a small Vim-style text editor\n");
    printf("i/a/o insert; Esc normal; hjkl/arrows move; x/dd delete; u undo\n"
           "/text search; n next; :w [file] save; :wq save and quit; :q! discard\n");
    return 0;
  }
  if (argc > 2)
  {
    ui::usage("usage: vim [file]\n");
    return 1;
  }
  void *storage = malloc(sizeof(vim::Editor));
  if (!storage)
  {
    ui::error("vim: out of memory\n");
    return 1;
  }
  auto *editor = new (storage) vim::Editor;
  int result = editor->run(argc == 2 ? argv[1] : nullptr);
  editor->~Editor();
  free(storage);
  return result;
}
