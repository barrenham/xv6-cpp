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


extern "C"
{

// Parsed command representation
#define EXEC 1
#define REDIR 2
#define PIPE 3
#define LIST 4
#define BACK 5

#define MAXARGS MAXARG

  struct cmd
  {
    int type;
  };

  struct execcmd
  {
    int type;
    char *argv[MAXARGS];
    char *eargv[MAXARGS];
  };

  struct redircmd
  {
    int type;
    struct cmd *cmd;
    char *file;
    char *efile;
    int mode;
    int fd;
  };

  struct pipecmd
  {
    int type;
    struct cmd *left;
    struct cmd *right;
  };

  struct listcmd
  {
    int type;
    struct cmd *left;
    struct cmd *right;
  };

  struct backcmd
  {
    int type;
    struct cmd *cmd;
  };

  bool parent_environment(char *line);
  int fork1(void); // Fork but panics on failure.
  void panic(char *);
  struct cmd *parsecmd(char *);
  void runcmd(struct cmd *) __attribute__((noreturn));

  static int child_status(int child)
  {
    int status = 0, result;
    do { result = waitpid(child, &status, 0); } while (result == -EINTR);
    if (result < 0) return 255;
    if (WIFSIGNALED(status)) return 128 + WTERMSIG(status);
    return WIFEXITED(status) ? WEXITSTATUS(status) : 255;
  }

  // Execute cmd.  Never returns.
  void runcmd(struct cmd *cmd)
  {
    int p[2];
    struct backcmd *bcmd;
    struct execcmd *ecmd;
    struct listcmd *lcmd;
    struct pipecmd *pcmd;
    struct redircmd *rcmd;

    if (cmd == 0)
    {
      exit(1);
    }

    switch (cmd->type)
    {
    default:
      panic("runcmd");

    case EXEC:
      ecmd = (struct execcmd *)cmd;
      if (ecmd->argv[0] == 0)
      {
        exit(1);
      }
      if (!shell::prepare(ecmd->argv)) { fprintf(2, "sh: environment expansion failed\n"); exit(1); }
      { int status = shell::builtin(ecmd->argv); if (status >= 0) exit(status); }
      // A list coordinator may reach its right-hand EXEC while ignoring SIGINT.
      sigdisposition(SIGINT, SIGNAL_DEFAULT);
      sigdisposition(SIGTSTP, SIGNAL_DEFAULT);
      sigdisposition(SIGTTIN, SIGNAL_DEFAULT);
      execvp(ecmd->argv[0], ecmd->argv);
      fprintf(2, "exec %s failed\n", ecmd->argv[0]);
      break;

    case REDIR:
      rcmd = (struct redircmd *)cmd;
      close(rcmd->fd);
      rcmd->file = shell::expand(rcmd->file);
      if (!rcmd->file || open(rcmd->file, rcmd->mode) < 0)
      {
        fprintf(2, "open %s failed\n", rcmd->file);
        exit(1);
      }
      runcmd(rcmd->cmd);
      break;

    case LIST:
      sigdisposition(SIGINT, SIGNAL_IGNORE);
      lcmd = (struct listcmd *)cmd;
      {
        bool handled = false;
        if (lcmd->left->type == EXEC)
        {
          auto *left = reinterpret_cast<execcmd *>(lcmd->left);
          unsigned first = 0; while (left->argv[first] && shell::assignment(left->argv[first])) ++first;
          const char *name = left->argv[first];
          if (!name || !strcmp(name, "export") || !strcmp(name, "unset") || !strcmp(name, "cd") || !strcmp(name, "umask"))
          { handled = true; if (!shell::prepare(left->argv) || shell::builtin(left->argv) != 0) fprintf(2, "sh: builtin failed\n"); }
        }
        if (!handled)
        {
          int child = fork1();
          if (child == 0)
          {
            sigdisposition(SIGINT, SIGNAL_DEFAULT); sigdisposition(SIGTSTP, SIGNAL_DEFAULT); sigdisposition(SIGTTIN, SIGNAL_DEFAULT);
            runcmd(lcmd->left);
          }
          int status = child_status(child); if (status >= 128) exit(status);
        }
      }
      runcmd(lcmd->right);
      break;

    case PIPE:
      sigdisposition(SIGINT, SIGNAL_IGNORE);
      pcmd = (struct pipecmd *)cmd;
      if (pipe(p) < 0)
      {
        panic("pipe");
      }
      if (fork1() == 0)
      {
        close(1);
        dup(p[1]);
        close(p[0]);
        close(p[1]);
        sigdisposition(SIGINT, SIGNAL_DEFAULT);
        sigdisposition(SIGTSTP, SIGNAL_DEFAULT);
        sigdisposition(SIGTTIN, SIGNAL_DEFAULT);
        runcmd(pcmd->left);
      }
      if (fork1() == 0)
      {
        close(0);
        dup(p[0]);
        close(p[0]);
        close(p[1]);
        sigdisposition(SIGINT, SIGNAL_DEFAULT);
        sigdisposition(SIGTSTP, SIGNAL_DEFAULT);
        sigdisposition(SIGTTIN, SIGNAL_DEFAULT);
        runcmd(pcmd->right);
      }
      close(p[0]);
      close(p[1]);
      {
        int first_status = child_status(-1);
        int second_status = child_status(-1);
        if (first_status >= 128)
        {
          exit(first_status);
        }
        if (second_status >= 128)
        {
          exit(second_status);
        }
      }
      break;

    case BACK:
      sigdisposition(SIGINT, SIGNAL_IGNORE);
      bcmd = (struct backcmd *)cmd;
      {
        int child = fork1();
        if (child == 0)
        {
          if (setpgid(0, 0) < 0)
          {
            exit(1);
          }
          sigdisposition(SIGINT, SIGNAL_DEFAULT);
          sigdisposition(SIGTSTP, SIGNAL_DEFAULT);
          sigdisposition(SIGTTIN, SIGNAL_DEFAULT);
          runcmd(bcmd->cmd);
        }
        setpgid(child, child);
      }
      break;
    }
    exit(0);
  }

  int getcmd(char *buf, int nbuf);

  static int child_pipe[2] = {-1, -1};
  static volatile sig_atomic_t children_changed = 0;

  static void child_signal(int)
  {
    children_changed = 1;
    // A full pipe already contains a wakeup. Never wait or touch the job list in
    // a handler; poll observes this byte even if the signal preceded its sleep.
    write(child_pipe[1], "c", 1);
  }

  static void reset_child_notifications()
  {
    sigdisposition(SIGCHLD, SIGNAL_DEFAULT);
    if (child_pipe[0] >= 0) close(child_pipe[0]);
    if (child_pipe[1] >= 0) close(child_pipe[1]);
    child_pipe[0] = child_pipe[1] = -1;
    children_changed = 0;
  }

  static void initialize_child_notifications()
  {
    if (pipe(child_pipe) < 0 ||
        fcntl(child_pipe[0], F_SETFL, O_NONBLOCK) < 0 ||
        fcntl(child_pipe[1], F_SETFL, O_NONBLOCK) < 0)
      panic("sh: cannot create child notification pipe");
    struct sigaction action{child_signal, 0, SA_RESTART};
    sigset_t set = 0;
    sigaddset(&set, SIGCHLD);
    if (sigaction(SIGCHLD, &action, nullptr) < 0 ||
        sigprocmask(SIG_UNBLOCK, &set, nullptr) < 0)
      panic("sh: cannot install child handler");
  }

  static int read_retry(int fd, void *data, int count)
  {
    int result;
    do { result = read(fd, data, count); } while (result == -EINTR);
    return result;
  }

  // Private descriptor remains usable when a command redirects standard streams.
  static int terminal_fd = -1;
  static int shell_group = -1;

  static void initialize_terminal()
  {
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

  static void remove_job(Job *job)
  {
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

  static void show_job(Job *job, const char *state)
  {
    char line[sizeof(job->command) + 48];
    unsigned used = 0;
    auto append = [&](const char *text)
    {
      while (*text && used < sizeof(line)) line[used++] = *text++;
    };
    append("[");
    char digits[12];
    unsigned count = 0, id = job->id;
    do { digits[count++] = '0' + id % 10; id /= 10; } while (id);
    while (count) line[used++] = digits[--count];
    append("] ");
    append(state);
    append(" ");
    append(job->command);
    append("\n");
    write(1, line, used);
  }

  static bool refresh_jobs(bool separate_line = false, Job *foreground = nullptr)
  {
    bool printed = false;
    for (Job *job = jobs_head, *next; job; job = next)
    {
      next = job->next;
      if (job == foreground) continue;
      int status;
      for (;;)
      {
        int pid = waitpid(job->pid, &status, WNOHANG | WUNTRACED | WCONTINUED);
        if (pid == -EINTR) continue;
        if (pid == 0)
        {
          break;
        }
        if (pid < 0 || WIFEXITED(status) || WIFSIGNALED(status))
        {
          if (separate_line && !printed) write(2, "\n", 1);
          show_job(job, "Done");
          printed = true;
          remove_job(job);
          break;
        }
        bool stopped = WIFSTOPPED(status);
        if (stopped != job->stopped)
        {
          if (separate_line && !printed) write(2, "\n", 1);
          show_job(job, stopped ? "Stopped" : "Running");
          printed = true;
        }
        job->stopped = stopped;
      }
    }
    return printed;
  }

  static bool service_children(bool separate_line = false, Job *foreground = nullptr)
  {
    if (!children_changed) return false;
    children_changed = 0;
    char bytes[64];
    for (;;)
    {
      int n = read(child_pipe[0], bytes, sizeof(bytes));
      if (n > 0 || n == -EINTR) continue;
      break;
    }
    return refresh_jobs(separate_line, foreground);
  }

  int getcmd(char *buf, int nbuf)
  {
    shell::Editor editor(buf, nbuf);
    shell::prompt();
    memset(buf, 0, nbuf);
    int used = 0;
    while (used + 1 < nbuf)
    {
      if (service_children(true))
      {
        if (editor.active()) editor.redraw(used);
        else { shell::prompt(); if (used) write(2, buf, used); }
      }
      pollfd events[] = {{0, POLLIN, 0}, {child_pipe[0], POLLIN, 0}};
      int ready = poll(events, 2, -1);
      if (ready == -EINTR) continue;
      if (ready < 0) return -1;
      if (events[1].revents) continue;
      if (!(events[0].revents & (POLLIN | POLLHUP | POLLERR))) continue;
      char byte;
      int n = read(0, &byte, 1);
      if (n == -EINTR || n == -EAGAIN) continue;
      if (n <= 0) return used ? 0 : -1;
      if (editor.active())
      { int action = editor.input(byte, used); if (action < 0) return -1; if (action > 0) break; continue; }
      buf[used++] = byte;
      if (byte == '\n' || byte == '\r') break;
    }
    return 0;
  }

  static void foreground_job(Job *job, bool resume, const termios *before = nullptr,
                             int before_flags = -1)
  {
    int shell_flags = before_flags >= 0 ? before_flags : fcntl(terminal_fd, F_GETFL);
    termios shell_modes;
    bool saved = before != nullptr || tcgetattr(terminal_fd, &shell_modes) == 0;
    if (before)
    {
      shell_modes = *before;
    }
    if (tcsetpgrp(terminal_fd, job->pid) < 0)
    {
      fprintf(2, "sh: cannot give terminal to job\n");
      return;
    }
    if (resume)
    {
      if (job->modes_valid)
      {
        tcsetattr(terminal_fd, TCSANOW, &job->modes);
      }
      show_job(job, "Foreground");
      if (job->flags >= 0) fcntl(terminal_fd, F_SETFL, job->flags);
      if (sigsend(-job->pid, SIGCONT) < 0)
      {
        tcsetpgrp(terminal_fd, shell_group);
        if (shell_flags >= 0) fcntl(terminal_fd, F_SETFL, shell_flags);
        if (saved) tcsetattr(terminal_fd, TCSANOW, &shell_modes);
        fprintf(2, "sh: cannot continue job\n");
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
      if (pid > 0 || pid == -1) break;
      if (pid == -EINTR) continue;
      // Drain notifications before checking status. A change after that check
      // leaves a byte for poll, closing the status-check-to-sleep window.
      pollfd event{child_pipe[0], POLLIN, 0};
      int ready = poll(&event, 1, -1);
      if (ready < 0 && ready != -EINTR) break;
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
    if (shell_flags >= 0) fcntl(terminal_fd, F_SETFL, shell_flags);
    if (saved && (job->stopped ||
                  (pid > 0 && (WIFSIGNALED(status) ||
                               (WIFEXITED(status) && WEXITSTATUS(status) >= 128)))))
    {
      tcsetattr(terminal_fd, TCSANOW, &shell_modes);
    }
    if (job->stopped)
    {
      show_job(job, "Stopped");
    }
    else
    {
      remove_job(job);
    }
  }

  static Job *select_job(const char *argument)
  {
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

  static bool job_builtin(char *text)
  {
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
    if (terminal_fd < 0)
    {
      fprintf(2, "sh: no interactive terminal\n");
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
        fprintf(2, "usage: jobs\n");
        return true;
      }
      for (Job *job = jobs_head; job; job = job->next)
      {
        show_job(job, job->stopped ? "Stopped" : "Running");
      }
      return true;
    }
    Job *job = select_job(end);
    if (!job)
    {
      fprintf(2, "sh: no such job\n");
      return true;
    }
    if (front)
    {
      foreground_job(job, true);
    }
    else
    {
      show_job(job, "Foreground");
      if (job->flags >= 0) fcntl(terminal_fd, F_SETFL, job->flags);
      if (sigsend(-job->pid, SIGCONT) < 0)
      {
        fprintf(2, "sh: cannot continue job\n");
      }
      else
      {
        job->stopped = false;
        show_job(job, "Running");
      }
    }
    return true;
  }

  static void launch_command(char *text)
  {
    if (terminal_fd < 0)
    {
      int child = fork1();
      if (child == 0)
      {
        runcmd(parsecmd(text));
      }
      while (waitpid(child, nullptr, 0) == -EINTR) service_children();
      return;
    }
    int length = strlen(text);
    while (length && (text[length - 1] == '\n' || text[length - 1] == ' ' ||
                      text[length - 1] == '\t'))
    {
      --length;
    }
    bool background = length && text[length - 1] == '&';
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
      fprintf(2, "sh: job IDs exhausted\n");
      return;
    }
    auto *job = static_cast<Job *>(malloc(sizeof(Job)));
    if (!job)
    {
      fprintf(2, "sh: no job memory\n");
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
      fprintf(2, "sh: fork failed\n");
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
      runcmd(parsecmd(text));
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
      while (write(gate[1], "g", 1) == -EINTR) {}
    }
    close(ready[0]);
    close(gate[1]);
    if (background)
    {
      show_job(job, "Running");
    }
    else
    {
      foreground_job(job, false, saved_modes ? &launch_modes : nullptr, launch_flags);
    }
  }

  int main(void)
  {
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
    initialize_terminal();
    initialize_child_notifications();
    while (getcmd(buf, sizeof(buf)) >= 0)
    {
      char *cmd = buf;
      while (*cmd == ' ' || *cmd == '\t')
      {
        cmd++;
      }
      if (*cmd == '\n')
      {
        continue;
      }
      if (job_builtin(cmd))
      {
        continue;
      }
      refresh_jobs();
      if (!parent_environment(cmd)) launch_command(cmd);
    }
    sigdisposition(SIGCHLD, SIGNAL_DEFAULT);
    for (Job *job = jobs_head; job; job = job->next)
    {
      sigsend(-job->pid, SIGKILL);
    }
    while (wait(nullptr) >= 0)
    {
    }
    exit(0);
  }

  bool parent_environment(char *line)
  {
    if (!shell::parent_candidate(line)) return false;
    char *copy = static_cast<char *>(malloc(strlen(line) + 1)); if (!copy) return true;
    strcpy(copy, line); auto *tree = parsecmd(copy);
    if (tree->type != EXEC) { free(copy); return false; }
    auto *command = reinterpret_cast<execcmd *>(tree);
    unsigned first = 0; while (command->argv[first] && shell::assignment(command->argv[first])) ++first;
    const char *name = command->argv[first];
    bool candidate = !name || !strcmp(name, "export") || !strcmp(name, "unset") || !strcmp(name, "cd") || !strcmp(name, "umask");
    if (candidate)
    {
      if (!shell::prepare(command->argv) || shell::builtin(command->argv) != 0) fprintf(2, "sh: invalid environment command\n");
      for (unsigned i = 0; command->argv[i]; ++i) free(command->argv[i]);
    }
    free(tree); free(copy); return candidate;
  }

  void panic(char *s)
  {
    fprintf(2, "%s\n", s);
    exit(1);
  }

  int fork1(void)
  {
    int pid;

    pid = fork();
    if (pid == -1)
    {
      panic("fork");
    }
    if (pid == 0) reset_child_notifications();
    return pid;
  }

  //PAGEBREAK!
  // Constructors

  struct cmd *execcmd(void)
  {
    struct execcmd *cmd;

    cmd = static_cast<struct execcmd *>(malloc(sizeof(*cmd)));
    memset(cmd, 0, sizeof(*cmd));
    cmd->type = EXEC;
    return (struct cmd *)cmd;
  }

  struct cmd *redircmd(struct cmd *subcmd, char *file, char *efile, int mode, int fd)
  {
    struct redircmd *cmd;

    cmd = static_cast<struct redircmd *>(malloc(sizeof(*cmd)));
    memset(cmd, 0, sizeof(*cmd));
    cmd->type = REDIR;
    cmd->cmd = subcmd;
    cmd->file = file;
    cmd->efile = efile;
    cmd->mode = mode;
    cmd->fd = fd;
    return (struct cmd *)cmd;
  }

  struct cmd *pipecmd(struct cmd *left, struct cmd *right)
  {
    struct pipecmd *cmd;

    cmd = static_cast<struct pipecmd *>(malloc(sizeof(*cmd)));
    memset(cmd, 0, sizeof(*cmd));
    cmd->type = PIPE;
    cmd->left = left;
    cmd->right = right;
    return (struct cmd *)cmd;
  }

  struct cmd *listcmd(struct cmd *left, struct cmd *right)
  {
    struct listcmd *cmd;

    cmd = static_cast<struct listcmd *>(malloc(sizeof(*cmd)));
    memset(cmd, 0, sizeof(*cmd));
    cmd->type = LIST;
    cmd->left = left;
    cmd->right = right;
    return (struct cmd *)cmd;
  }

  struct cmd *backcmd(struct cmd *subcmd)
  {
    struct backcmd *cmd;

    cmd = static_cast<struct backcmd *>(malloc(sizeof(*cmd)));
    memset(cmd, 0, sizeof(*cmd));
    cmd->type = BACK;
    cmd->cmd = subcmd;
    return (struct cmd *)cmd;
  }

  //PAGEBREAK!
  // Parsing

  char whitespace[] = " \t\r\n\v";
  char symbols[] = "<|>&;()";

  int gettoken(char **ps, char *es, char **q, char **eq)
  {
    char *s;
    int ret;

    s = *ps;
    while (s < es && strchr(whitespace, *s))
    {
      s++;
    }
    if (q)
    {
      *q = s;
    }
    ret = *s;
    switch (*s)
    {
    case 0:
      break;
    case '|':
    case '(':
    case ')':
    case ';':
    case '&':
    case '<':
      s++;
      break;
    case '>':
      s++;
      if (*s == '>')
      {
        ret = '+';
        s++;
      }
      break;
    default:
      ret = 'a';
      { char quote = 0;
        while (s < es && (quote || (!strchr(whitespace, *s) && !strchr(symbols, *s))))
        {
          if (*s == '\\' && quote != '\'' && s + 1 < es) { s += 2; continue; }
          if ((*s == '\'' || *s == '"') && (!quote || quote == *s)) quote = quote ? 0 : *s;
          ++s;
        }
        if (quote) panic("unterminated quote");
      }
      break;
    }
    if (eq)
    {
      *eq = s;
    }

    while (s < es && strchr(whitespace, *s))
    {
      s++;
    }
    *ps = s;
    return ret;
  }

  int peek(char **ps, char *es, char *toks)
  {
    char *s;

    s = *ps;
    while (s < es && strchr(whitespace, *s))
    {
      s++;
    }
    *ps = s;
    return *s && strchr(toks, *s);
  }

  struct cmd *parseline(char **, char *);
  struct cmd *parsepipe(char **, char *);
  struct cmd *parseexec(char **, char *);
  struct cmd *nulterminate(struct cmd *);

  struct cmd *parsecmd(char *s)
  {
    char *es;
    struct cmd *cmd;

    es = s + strlen(s);
    cmd = parseline(&s, es);
    peek(&s, es, "");
    if (s != es)
    {
      fprintf(2, "leftovers: %s\n", s);
      panic("syntax");
    }
    nulterminate(cmd);
    return cmd;
  }

  struct cmd *parseline(char **ps, char *es)
  {
    struct cmd *cmd;

    cmd = parsepipe(ps, es);
    while (peek(ps, es, "&"))
    {
      gettoken(ps, es, 0, 0);
      cmd = backcmd(cmd);
    }
    if (peek(ps, es, ";"))
    {
      gettoken(ps, es, 0, 0);
      cmd = listcmd(cmd, parseline(ps, es));
    }
    return cmd;
  }

  struct cmd *parsepipe(char **ps, char *es)
  {
    struct cmd *cmd;

    cmd = parseexec(ps, es);
    if (peek(ps, es, "|"))
    {
      gettoken(ps, es, 0, 0);
      cmd = pipecmd(cmd, parsepipe(ps, es));
    }
    return cmd;
  }

  struct cmd *parseredirs(struct cmd *cmd, char **ps, char *es)
  {
    int tok;
    char *q, *eq;

    while (peek(ps, es, "<>"))
    {
      tok = gettoken(ps, es, 0, 0);
      if (gettoken(ps, es, &q, &eq) != 'a')
      {
        panic("missing file for redirection");
      }
      switch (tok)
      {
      case '<':
        cmd = redircmd(cmd, q, eq, O_RDONLY, 0);
        break;
      case '>':
        cmd = redircmd(cmd, q, eq, O_WRONLY | O_CREATE | O_TRUNC, 1);
        break;
      case '+': // >>
        cmd = redircmd(cmd, q, eq, O_WRONLY | O_CREATE, 1);
        break;
      }
    }
    return cmd;
  }

  struct cmd *parseblock(char **ps, char *es)
  {
    struct cmd *cmd;

    if (!peek(ps, es, "("))
    {
      panic("parseblock");
    }
    gettoken(ps, es, 0, 0);
    cmd = parseline(ps, es);
    if (!peek(ps, es, ")"))
    {
      panic("syntax - missing )");
    }
    gettoken(ps, es, 0, 0);
    cmd = parseredirs(cmd, ps, es);
    return cmd;
  }

  struct cmd *parseexec(char **ps, char *es)
  {
    char *q, *eq;
    int tok, argc;
    struct execcmd *cmd;
    struct cmd *ret;

    if (peek(ps, es, "("))
    {
      return parseblock(ps, es);
    }

    ret = execcmd();
    cmd = (struct execcmd *)ret;

    argc = 0;
    ret = parseredirs(ret, ps, es);
    while (!peek(ps, es, "|)&;"))
    {
      if ((tok = gettoken(ps, es, &q, &eq)) == 0)
      {
        break;
      }
      if (tok != 'a')
      {
        panic("syntax");
      }
      cmd->argv[argc] = q;
      cmd->eargv[argc] = eq;
      argc++;
      if (argc >= MAXARGS)
      {
        panic("too many args");
      }
      ret = parseredirs(ret, ps, es);
    }
    cmd->argv[argc] = 0;
    cmd->eargv[argc] = 0;
    return ret;
  }

  // NUL-terminate all the counted strings.
  struct cmd *nulterminate(struct cmd *cmd)
  {
    int i;
    struct backcmd *bcmd;
    struct execcmd *ecmd;
    struct listcmd *lcmd;
    struct pipecmd *pcmd;
    struct redircmd *rcmd;

    if (cmd == 0)
    {
      return 0;
    }

    switch (cmd->type)
    {
    case EXEC:
      ecmd = (struct execcmd *)cmd;
      for (i = 0; ecmd->argv[i]; i++)
      {
        *ecmd->eargv[i] = 0;
      }
      break;

    case REDIR:
      rcmd = (struct redircmd *)cmd;
      nulterminate(rcmd->cmd);
      *rcmd->efile = 0;
      break;

    case PIPE:
      pcmd = (struct pipecmd *)cmd;
      nulterminate(pcmd->left);
      nulterminate(pcmd->right);
      break;

    case LIST:
      lcmd = (struct listcmd *)cmd;
      nulterminate(lcmd->left);
      nulterminate(lcmd->right);
      break;

    case BACK:
      bcmd = (struct backcmd *)cmd;
      nulterminate(bcmd->cmd);
      break;
    }
    return cmd;
  }
}
