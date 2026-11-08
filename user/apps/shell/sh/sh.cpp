#include "user/runtime/ui.h"
#include "application.h"
// Shell.

#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "user/runtime/user.h"
#include "kernel/fs/fcntl.h"
#include "kernel/fs/io_status.h"
#include "kernel/fs/poll.h"
#include "kernel/tty/termios.h"
#include "kernel/process/signals/signal.h"
#include "kernel/process/wait/wait.h"
#include "environment.h"
#include "editor.h"

#include "command.h"

extern "C"
{
  int getcmd(char *buf, int nbuf);

  static int child_pipe[2] = {-1, -1};
  static volatile sig_atomic_t children_changed = 0;

  static void
  child_signal(int) {
    children_changed = 1;
    // A full pipe already contains a wakeup. Never wait or touch the job list in
    // a handler; poll observes this byte even if the signal preceded its sleep.
    write(child_pipe[1], "c", 1);
  }

  static void
  reset_child_notifications() {
    sigdisposition(SIGCHLD, SIGNAL_DEFAULT);
    if (child_pipe[0] >= 0)
    {
      close(child_pipe[0]);
    }
    if (child_pipe[1] >= 0)
    {
      close(child_pipe[1]);
    }
    child_pipe[0] = child_pipe[1] = -1;
    children_changed = 0;
  }

  static void
  initialize_child_notifications() {
    if (pipe(child_pipe) < 0 || fcntl(child_pipe[0], F_SETFL, O_NONBLOCK) < 0 ||
        fcntl(child_pipe[1], F_SETFL, O_NONBLOCK) < 0)
    {
      panic("sh: cannot create child notification pipe");
    }
    struct sigaction action{child_signal, 0, SA_RESTART};
    sigset_t set = 0;
    sigaddset(&set, SIGCHLD);
    if (sigaction(SIGCHLD, &action, nullptr) < 0 ||
        sigprocmask(SIG_UNBLOCK, &set, nullptr) < 0)
    {
      panic("sh: cannot install child handler");
    }
  }

  static int
  read_retry(int fd, void *data, int count) {
    int result;
    do
    {
      result = read(fd, data, count);
    } while ((result < 0 && errno == EINTR));
    return result;
  }

  // Private descriptor remains usable when a command redirects standard streams.
  static int terminal_fd = -1;
  static int input_fd = 0;
  static int shell_group = -1;

  static void
  initialize_terminal() {
    termios settings;
    if (tcgetattr(0, &settings) < 0)
    {
      return; // command-input pipe: noninteractive
    }
    terminal_fd = dup(0);
    if (terminal_fd < 0)
    {
      panic("sh: no terminal descriptor");
    }
    if (tcgetpgrp(terminal_fd) < 0)
    {
      if (setsid() < 0 || ioctl(terminal_fd, TIOCSCTTY, 0) < 0)
      {
        panic("sh: cannot acquire controlling terminal");
      }
    }
    if (getpgrp() != getpid() && setpgid(0, 0) < 0)
    {
      panic("sh: cannot create shell group");
    }
    shell_group = getpgrp();
    sigdisposition(SIGTSTP, SIGNAL_IGNORE);
    sigdisposition(SIGTTIN, SIGNAL_IGNORE);
    if (sigdisposition(SIGINT, SIGNAL_IGNORE) < 0 ||
        tcsetpgrp(terminal_fd, shell_group) < 0)
    {
      panic("sh: cannot own terminal");
    }
  }

  struct Job
  {
    int id;
    int pid;
    bool stopped;
    bool modes_valid;
    int flags;
    termios modes;
    char command[512];
    Job *next;
  };

  static Job *jobs_head = nullptr;
  static unsigned next_job = 1;

  static void
  remove_job(Job *job) {
    Job **link = &jobs_head;
    while (*link && *link != job)
    {
      link = &(*link)->next;
    }
    if (*link)
    {
      *link = job->next;
    }
    free(job);
  }

  static void
  show_job(Job *job, const char *state) {
    char line[sizeof(job->command) + 48];
    unsigned used = 0;
    auto append = [&](const char *text) {
      while (*text && used + 1 < sizeof(line))
      {
        line[used++] = *text++;
      }
    };
    append("[");
    char digits[12];
    unsigned count = 0, id = job->id;
    do
    {
      digits[count++] = '0' + id % 10;
      id /= 10;
    } while (id);
    while (count)
    {
      line[used++] = digits[--count];
    }
    append("] ");
    append(state);
    append(" ");
    append(job->command);
    append("\n");
    line[used] = 0;
    ui::print(1, !strcmp(state, "Stopped") ? ui::Tone::Warning : ui::Tone::Info, "%s",
              line);
  }

  static bool
  refresh_jobs(bool separate_line = false, Job *foreground = nullptr,
               shell::Editor *editor = nullptr) {
    bool printed = false;
    for (Job *job = jobs_head, *next; job; job = next)
    {
      next = job->next;
      if (job == foreground)
      {
        continue;
      }
      int status;
      for (;;)
      {
        int pid = waitpid(job->pid, &status, WNOHANG | WUNTRACED | WCONTINUED);
        if ((pid < 0 && errno == EINTR))
        {
          continue;
        }
        if (pid == 0)
        {
          break;
        }
        if (pid < 0 || WIFEXITED(status) || WIFSIGNALED(status))
        {
          if (!printed && editor)
          {
            editor->begin_notice();
          }
          else if (separate_line && !printed)
          {
            write(2, "\n", 1);
          }
          show_job(job, "Done");
          printed = true;
          remove_job(job);
          break;
        }
        bool stopped = WIFSTOPPED(status);
        if (stopped != job->stopped)
        {
          if (!printed && editor)
          {
            editor->begin_notice();
          }
          else if (separate_line && !printed)
          {
            write(2, "\n", 1);
          }
          show_job(job, stopped ? "Stopped" : "Running");
          printed = true;
        }
        job->stopped = stopped;
      }
    }
    return printed;
  }

  static bool
  service_children(bool separate_line = false, Job *foreground = nullptr,
                   shell::Editor *editor = nullptr) {
    if (!children_changed)
    {
      return false;
    }
    children_changed = 0;
    char bytes[64];
    for (;;)
    {
      int n = read(child_pipe[0], bytes, sizeof(bytes));
      if (n > 0 || (n < 0 && errno == EINTR))
      {
        continue;
      }
      break;
    }
    return refresh_jobs(separate_line, foreground, editor);
  }

  int
  getcmd(char *buf, int nbuf) {
    if (terminal_fd >= 0)
    {
      shell::refresh_prompt();
    }
    shell::Editor editor(buf, nbuf, terminal_fd >= 0);
    if (terminal_fd >= 0)
    {
      shell::prompt();
    }
    memset(buf, 0, nbuf);
    int used = 0;
    unsigned burst = 0;
    while (used + 1 < nbuf)
    {
      pollfd events[] = {{input_fd, POLLIN, 0}, {child_pipe[0], POLLIN, 0}};
      int ready = poll(events, 2, editor.pending() || children_changed ? 0 : -1);
      if (ready < 0 && errno == EINTR)
      {
        continue;
      }
      if (ready < 0)
      {
        return -1;
      }
      // Drain available input before notifications redraw the prompt. Read one
      // byte at a time so Enter never steals input intended for the next app.
      if (!(events[0].revents & (POLLIN | POLLHUP | POLLERR)) || burst == 128)
      {
        editor.flush(used);
        burst = 0;
        if (service_children(true, nullptr, editor.active() ? &editor : nullptr))
        {
          if (editor.active())
          {
            editor.redraw(used, true);
          }
          else
          {
            shell::prompt();
            if (used)
            {
              write(2, buf, used);
            }
          }
        }
        continue;
      }
      ++burst;
      char byte;
      int n = read(input_fd, &byte, 1);
      if ((n < 0 && errno == EINTR) || (n < 0 && errno == EAGAIN))
      {
        continue;
      }
      if (n <= 0)
      {
        return used ? 0 : -1;
      }
      if (editor.active())
      {
        int action = editor.input(byte, used);
        if (action < 0)
        {
          return -1;
        }
        if (action > 0)
        {
          break;
        }
        continue;
      }
      buf[used++] = byte;
      if (byte == '\n' || byte == '\r')
      {
        break;
      }
    }
    return 0;
  }

  static void
  foreground_job(Job *job, bool resume, const termios *before = nullptr,
                 int before_flags = -1) {
    int shell_flags = before_flags >= 0 ? before_flags : fcntl(terminal_fd, F_GETFL);
    termios shell_modes;
    bool saved = before != nullptr || tcgetattr(terminal_fd, &shell_modes) == 0;
    if (before)
    {
      shell_modes = *before;
    }
    if (tcsetpgrp(terminal_fd, job->pid) < 0)
    {
      ui::error("sh: cannot give terminal to job\n");
      return;
    }
    if (resume)
    {
      if (job->modes_valid)
      {
        tcsetattr(terminal_fd, TCSANOW, &job->modes);
      }
      show_job(job, "Foreground");
      if (job->flags >= 0)
      {
        fcntl(terminal_fd, F_SETFL, job->flags);
      }
      if (sigsend(-job->pid, SIGCONT) < 0)
      {
        tcsetpgrp(terminal_fd, shell_group);
        if (shell_flags >= 0)
        {
          fcntl(terminal_fd, F_SETFL, shell_flags);
        }
        if (saved)
        {
          tcsetattr(terminal_fd, TCSANOW, &shell_modes);
        }
        ui::error("sh: cannot continue job\n");
        return;
      }
    }
    job->stopped = false;
    int status = 0;
    int pid;
    for (;;)
    {
      service_children(false, job);
      pid = waitpid(job->pid, &status, WNOHANG | WUNTRACED);
      if (pid > 0 || pid == -1)
      {
        break;
      }
      if ((pid < 0 && errno == EINTR))
      {
        continue;
      }
      // Drain notifications before checking status. A change after that check
      // leaves a byte for poll, closing the status-check-to-sleep window.
      pollfd event{child_pipe[0], POLLIN, 0};
      int ready = poll(&event, 1, -1);
      if (ready < 0 && !(ready < 0 && errno == EINTR))
      {
        break;
      }
    }
    if (pid > 0 && WIFSTOPPED(status))
    {
      job->stopped = true;
      job->flags = fcntl(terminal_fd, F_GETFL);
      job->modes_valid = tcgetattr(terminal_fd, &job->modes) == 0;
    }
    if (tcsetpgrp(terminal_fd, shell_group) < 0)
    {
      panic("sh: cannot reclaim terminal");
    }
    if (shell_flags >= 0)
    {
      fcntl(terminal_fd, F_SETFL, shell_flags);
    }
    if (saved && (job->stopped ||
                  (pid > 0 && (WIFSIGNALED(status) ||
                               (WIFEXITED(status) && WEXITSTATUS(status) >= 128)))))
    {
      tcsetattr(terminal_fd, TCSANOW, &shell_modes);
    }
    shell::last_status = job->stopped          ? 128 + SIGTSTP
                         : pid < 0             ? 255
                         : WIFSIGNALED(status) ? 128 + WTERMSIG(status)
                                               : WEXITSTATUS(status);
    if (job->stopped)
    {
      show_job(job, "Stopped");
    }
    else
    {
      remove_job(job);
    }
  }

  static Job *
  select_job(const char *argument) {
    if (!argument || !*argument || *argument == '\n')
    {
      return jobs_head;
    }
    if (*argument == '%')
    {
      ++argument;
    }
    unsigned id = 0;
    if (*argument < '0' || *argument > '9')
    {
      return nullptr;
    }
    while (*argument >= '0' && *argument <= '9')
    {
      unsigned digit = *argument++ - '0';
      if (id > (0x7fffffffU - digit) / 10)
      {
        return nullptr;
      }
      id = id * 10 + digit;
    }
    while (*argument == ' ' || *argument == '\t')
    {
      ++argument;
    }
    if (*argument && *argument != '\n')
    {
      return nullptr;
    }
    for (Job *job = jobs_head; job; job = job->next)
    {
      if (job->id == static_cast<int>(id))
      {
        return job;
      }
    }
    return nullptr;
  }

  static bool
  job_builtin(char *text) {
    char *end = text;
    while (*end && *end != ' ' && *end != '\t' && *end != '\n')
    {
      ++end;
    }
    int length = end - text;
    bool list = length == 4 && memcmp(text, "jobs", 4) == 0;
    bool front = length == 2 && memcmp(text, "fg", 2) == 0;
    bool back = length == 2 && memcmp(text, "bg", 2) == 0;
    if (!list && !front && !back)
    {
      return false;
    }
    shell::last_status = 1;
    if (terminal_fd < 0)
    {
      ui::error("sh: no interactive terminal\n");
      return true;
    }
    while (*end == ' ' || *end == '\t')
    {
      ++end;
    }
    refresh_jobs();
    if (list)
    {
      if (*end && *end != '\n')
      {
        ui::usage("usage: jobs\n");
        return true;
      }
      for (Job *job = jobs_head; job; job = job->next)
      {
        show_job(job, job->stopped ? "Stopped" : "Running");
      }
      shell::last_status = 0;
      return true;
    }
    Job *job = select_job(end);
    if (!job)
    {
      ui::error("sh: no such job\n");
      return true;
    }
    if (front)
    {
      foreground_job(job, true);
    }
    else
    {
      show_job(job, "Foreground");
      if (job->flags >= 0)
      {
        fcntl(terminal_fd, F_SETFL, job->flags);
      }
      if (sigsend(-job->pid, SIGCONT) < 0)
      {
        ui::error("sh: cannot continue job\n");
      }
      else
      {
        job->stopped = false;
        shell::last_status = 0;
        show_job(job, "Running");
      }
    }
    return true;
  }

  static void
  launch_command(char *text, struct cmd *prepared = nullptr) {
    if (terminal_fd < 0)
    {
      int child = fork1();
      if (child == 0)
      {
        if (input_fd != 0)
        {
          close(input_fd);
        }
        runcmd(prepared ? prepared : parsecmd(text));
      }
      shell::last_status = child_status(child);
      return;
    }
    int length = strlen(text);
    while (length && (text[length - 1] == '\n' || text[length - 1] == ' ' ||
                      text[length - 1] == '\t'))
    {
      --length;
    }
    bool background =
        prepared ? prepared->type == BACK : length && text[length - 1] == '&';
    if (background)
    {
      --length;
    }
    while (length && (text[length - 1] == ' ' || text[length - 1] == '\t'))
    {
      --length;
    }
    text[length] = 0;
    if (!length)
    {
      return;
    }
    if (next_job > 0x7fffffffU)
    {
      ui::error("sh: job IDs exhausted\n");
      return;
    }
    auto *job = static_cast<Job *>(malloc(sizeof(Job)));
    if (!job)
    {
      ui::error("sh: no job memory\n");
      return;
    }
    memset(job, 0, sizeof(*job));
    job->flags = -1;
    strcpy(job->command, text);
    int ready[2], gate[2];
    if (pipe(ready) < 0)
    {
      free(job);
      return;
    }
    if (pipe(gate) < 0)
    {
      close(ready[0]);
      close(ready[1]);
      free(job);
      return;
    }
    int child = fork();
    if (child < 0)
    {
      close(ready[0]);
      close(ready[1]);
      close(gate[0]);
      close(gate[1]);
      free(job);
      ui::error("sh: fork failed\n");
      return;
    }
    if (child == 0)
    {
      reset_child_notifications();
      close(ready[0]);
      close(gate[1]);
      if (setpgid(0, 0) < 0)
      {
        exit(1);
      }
      sigdisposition(SIGINT, SIGNAL_DEFAULT);
      sigdisposition(SIGTSTP, SIGNAL_DEFAULT);
      sigdisposition(SIGTTIN, SIGNAL_DEFAULT);
      char byte = 'r';
      if (write(ready[1], &byte, 1) != 1)
      {
        exit(1);
      }
      close(ready[1]);
      if (read(gate[0], &byte, 1) != 1)
      {
        exit(1);
      }
      close(gate[0]);
      close(terminal_fd);
      runcmd(prepared
                 ? (prepared->type == BACK ? reinterpret_cast<backcmd *>(prepared)->cmd
                                           : prepared)
                 : parsecmd(text));
    }
    close(ready[1]);
    close(gate[0]);
    char byte;
    bool ready_ok = read_retry(ready[0], &byte, 1) == 1 && setpgid(child, child) == 0;
    job->id = next_job++;
    job->pid = child;
    job->next = jobs_head;
    jobs_head = job;
    int launch_flags = fcntl(terminal_fd, F_GETFL);
    termios launch_modes;
    bool saved_modes = tcgetattr(terminal_fd, &launch_modes) == 0;
    if (ready_ok && (background || tcsetpgrp(terminal_fd, child) == 0))
    {
      while ((write(gate[1], "g", 1) < 0 && errno == EINTR))
      {
      }
    }
    close(ready[0]);
    close(gate[1]);
    if (background)
    {
      show_job(job, "Running");
      shell::last_status = 0;
    }
    else
    {
      foreground_job(job, false, saved_modes ? &launch_modes : nullptr, launch_flags);
    }
  }

  static void
  free_command(struct cmd *tree) {
    if (!tree)
    {
      return;
    }
    if (tree->type == REDIR)
    {
      free_command(reinterpret_cast<redircmd *>(tree)->cmd);
    }
    else if (tree->type == BACK)
    {
      free_command(reinterpret_cast<backcmd *>(tree)->cmd);
    }
    else if (tree->type == PIPE || tree->type == LIST || tree->type == AND ||
             tree->type == OR)
    {
      auto *pair = reinterpret_cast<listcmd *>(tree);
      free_command(pair->left);
      free_command(pair->right);
    }
    free(tree);
  }

  static int
  evaluate(struct cmd *tree, char *display) {
    if (tree->type == LIST || tree->type == AND || tree->type == OR)
    {
      auto *pair = reinterpret_cast<listcmd *>(tree);
      int status = evaluate(pair->left, display);
      if (shell::exit_requested || status >= 128 ||
          (tree->type == AND && status != 0) || (tree->type == OR && status == 0))
      {
        return status;
      }
      return evaluate(pair->right, display);
    }
    if (tree->type == EXEC)
    {
      auto *command = reinterpret_cast<execcmd *>(tree);
      if (!command->argv[0])
      {
        return shell::last_status;
      }
      unsigned first = 0;
      while (command->argv[first] && shell::assignment(command->argv[first]))
      {
        ++first;
      }
      char *name = command->argv[first] ? shell::expand(command->argv[first]) : nullptr;
      bool job =
          name && (!strcmp(name, "jobs") || !strcmp(name, "fg") || !strcmp(name, "bg"));
      if (job)
      {
        free(name);
        if (!shell::prepare(command->argv))
        {
          return shell::last_status = 1;
        }
        auto *text = static_cast<char *>(malloc(512));
        unsigned used = 0;
        bool valid = text != nullptr;
        if (text)
        {
          text[0] = 0;
        }
        for (unsigned i = 0; command->argv[i]; ++i)
        {
          unsigned size = strlen(command->argv[i]);
          if (valid && used + size + 2 < 512)
          {
            if (used)
            {
              text[used++] = ' ';
            }
            strcpy(text + used, command->argv[i]);
            used += size;
          }
          else
          {
            valid = false;
          }
          free(command->argv[i]);
        }
        if (valid)
        {
          job_builtin(text);
        }
        else
        {
          shell::last_status = 1;
        }
        free(text);
        return shell::last_status;
      }
      bool builtin = !command->argv[first] ||
                     (name && (!strcmp(name, "cd") || !strcmp(name, "export") ||
                               !strcmp(name, "unset") || !strcmp(name, "umask") ||
                               !strcmp(name, "exit")));
      free(name);
      if (builtin)
      {
        shell::last_status =
            !shell::prepare(command->argv) ? 1 : shell::builtin(command->argv);
        for (unsigned i = 0; command->argv[i]; ++i)
        {
          free(command->argv[i]);
        }
        return shell::last_status;
      }
    }
    launch_command(display, tree);
    return shell::last_status;
  }

  static void
  strip_comment(char *line) {
    char quote = 0;
    for (char *p = line; *p; ++p)
    {
      if (*p == '\\' && quote != '\'' && p[1])
      {
        ++p;
        continue;
      }
      if ((*p == '\'' || *p == '"') && (!quote || *p == quote))
      {
        quote = quote ? 0 : *p;
      }
      else if (!quote && *p == '#' && (p == line || p[-1] == ' ' || p[-1] == '\t'))
      {
        *p = 0;
        break;
      }
    }
  }

  int
  app_main(int argc, char **argv) {
    static char buf[512];
    int fd;
    while ((fd = open("/dev/console", O_RDWR)) >= 0)
    {
      if (fd >= 3)
      {
        close(fd);
        break;
      }
    }
    if (argc > 2)
    {
      ui::usage("usage: sh [script]\n");
      exit(2);
    }
    if (argc == 2)
    {
      int script = open(argv[1], O_RDONLY);
      if (script < 0)
      {
        ui::error("sh: cannot open %s\n", argv[1]);
        exit(1);
      }
      input_fd = script;
    }
    if (input_fd == 0)
    {
      initialize_terminal();
    }
    initialize_child_notifications();
    while (!shell::exit_requested && getcmd(buf, sizeof(buf)) >= 0)
    {
      strip_comment(buf);
      char *cmd = buf;
      while (*cmd == ' ' || *cmd == '\t')
      {
        cmd++;
      }
      if (!*cmd || *cmd == '\n')
      {
        continue;
      }
      refresh_jobs();
      char *display = static_cast<char *>(malloc(strlen(cmd) + 1));
      if (!display)
      {
        shell::last_status = 1;
        continue;
      }
      strcpy(display, cmd);
      // A syntax error must not terminate the interactive login shell.
      int parser = fork1();
      if (!parser)
      {
        if (input_fd != 0)
        {
          close(input_fd);
        }
        parsecmd(cmd);
        exit(0);
      }
      if (child_status(parser) != 0)
      {
        shell::last_status = 2;
        free(display);
        if (input_fd != 0)
        {
          break;
        }
        continue;
      }
      auto *tree = parsecmd(cmd);
      evaluate(tree, display);
      free_command(tree);
      free(display);
    }
    if (input_fd != 0)
    {
      close(input_fd);
    }
    sigdisposition(SIGCHLD, SIGNAL_DEFAULT);
    for (Job *job = jobs_head; job; job = job->next)
    {
      sigsend(-job->pid, SIGKILL);
    }
    while (wait(nullptr) >= 0)
    {
    }
    exit(shell::last_status);
  }

  bool
  parent_environment(char *line) {
    if (!shell::parent_candidate(line))
    {
      return false;
    }
    char *copy = static_cast<char *>(malloc(strlen(line) + 1));
    if (!copy)
    {
      return true;
    }
    strcpy(copy, line);
    auto *tree = parsecmd(copy);
    if (tree->type != EXEC)
    {
      free(copy);
      return false;
    }
    auto *command = reinterpret_cast<execcmd *>(tree);
    unsigned first = 0;
    while (command->argv[first] && shell::assignment(command->argv[first]))
    {
      ++first;
    }
    const char *name = command->argv[first];
    bool candidate = !name || !strcmp(name, "export") || !strcmp(name, "unset") ||
                     !strcmp(name, "cd") || !strcmp(name, "umask");
    if (candidate)
    {
      if (!shell::prepare(command->argv) || shell::builtin(command->argv) != 0)
      {
        ui::error("sh: invalid environment command\n");
      }
      for (unsigned i = 0; command->argv[i]; ++i)
      {
        free(command->argv[i]);
      }
    }
    free(tree);
    free(copy);
    return candidate;
  }

  void
  panic(char *s) {
    ui::error("%s\n", s);
    exit(1);
  }

  int
  fork1(void) {
    int pid;

    pid = fork();
    if (pid == -1)
    {
      panic("fork");
    }
    if (pid == 0)
    {
      reset_child_notifications();
    }
    return pid;
  }
}
