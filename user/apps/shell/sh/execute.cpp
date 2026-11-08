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
  int
  child_status(int child) {
    int status = 0, result;
    do
    {
      result = waitpid(child, &status, 0);
    } while ((result < 0 && errno == EINTR));
    if (result < 0)
    {
      return 255;
    }
    if (WIFSIGNALED(status))
    {
      return 128 + WTERMSIG(status);
    }
    return WIFEXITED(status) ? WEXITSTATUS(status) : 255;
  }

  // Execute cmd.  Never returns.
  void
  runcmd(struct cmd *cmd) {
    int p[2], members[2];
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
        exit(shell::last_status);
      }
      if (!shell::prepare(ecmd->argv))
      {
        ui::error("sh: environment expansion failed\n");
        exit(1);
      }
      {
        int status = shell::builtin(ecmd->argv);
        if (status >= 0)
        {
          exit(status);
        }
      }
      // A list coordinator may reach its right-hand EXEC while ignoring SIGINT.
      sigdisposition(SIGINT, SIGNAL_DEFAULT);
      sigdisposition(SIGTSTP, SIGNAL_DEFAULT);
      sigdisposition(SIGTTIN, SIGNAL_DEFAULT);
      execvp(ecmd->argv[0], ecmd->argv);
      ui::error("exec %s failed\n", ecmd->argv[0]);
      exit(127);

    case REDIR:
      rcmd = (struct redircmd *)cmd;
      close(rcmd->fd);
      rcmd->file = shell::expand(rcmd->file);
      if (!rcmd->file || open(rcmd->file, rcmd->mode) < 0)
      {
        ui::error("open %s failed\n", rcmd->file);
        exit(1);
      }
      runcmd(rcmd->cmd);
      break;

    case AND:
    case OR:
    case LIST:
      sigdisposition(SIGINT, SIGNAL_IGNORE);
      lcmd = (struct listcmd *)cmd;
      {
        bool handled = false;
        if (lcmd->left->type == EXEC)
        {
          auto *left = reinterpret_cast<execcmd *>(lcmd->left);
          unsigned first = 0;
          while (left->argv[first] && shell::assignment(left->argv[first]))
          {
            ++first;
          }
          const char *name = left->argv[first];
          if (!name || !strcmp(name, "export") || !strcmp(name, "unset") ||
              !strcmp(name, "cd") || !strcmp(name, "umask") || !strcmp(name, "exit"))
          {
            handled = true;
            shell::last_status =
                !shell::prepare(left->argv) ? 1 : shell::builtin(left->argv);
          }
        }
        if (!handled)
        {
          int child = fork1();
          if (child == 0)
          {
            sigdisposition(SIGINT, SIGNAL_DEFAULT);
            sigdisposition(SIGTSTP, SIGNAL_DEFAULT);
            sigdisposition(SIGTTIN, SIGNAL_DEFAULT);
            runcmd(lcmd->left);
          }
          int status = child_status(child);
          shell::last_status = status;
        }
      }
      if (shell::exit_requested || shell::last_status >= 128 ||
          (cmd->type == AND && shell::last_status != 0) ||
          (cmd->type == OR && shell::last_status == 0))
      {
        exit(shell::last_status);
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
      if ((members[0] = fork1()) == 0)
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
      if ((members[1] = fork1()) == 0)
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
        int first_status = child_status(members[0]);
        int second_status = child_status(members[1]);
        if (first_status >= 128)
        {
          exit(first_status);
        }
        if (second_status >= 128)
        {
          exit(second_status);
        }
        exit(second_status);
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
}
