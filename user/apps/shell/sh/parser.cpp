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
  //PAGEBREAK!
  // Constructors

  struct cmd *
  execcmd(void) {
    struct execcmd *cmd;

    cmd = static_cast<struct execcmd *>(malloc(sizeof(*cmd)));
    memset(cmd, 0, sizeof(*cmd));
    cmd->type = EXEC;
    return (struct cmd *)cmd;
  }

  struct cmd *
  redircmd(struct cmd *subcmd, char *file, char *efile, int mode, int fd) {
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

  struct cmd *
  pipecmd(struct cmd *left, struct cmd *right) {
    struct pipecmd *cmd;

    cmd = static_cast<struct pipecmd *>(malloc(sizeof(*cmd)));
    memset(cmd, 0, sizeof(*cmd));
    cmd->type = PIPE;
    cmd->left = left;
    cmd->right = right;
    return (struct cmd *)cmd;
  }

  struct cmd *
  listcmd(struct cmd *left, struct cmd *right) {
    struct listcmd *cmd;

    cmd = static_cast<struct listcmd *>(malloc(sizeof(*cmd)));
    memset(cmd, 0, sizeof(*cmd));
    cmd->type = LIST;
    cmd->left = left;
    cmd->right = right;
    return (struct cmd *)cmd;
  }

  struct cmd *
  backcmd(struct cmd *subcmd) {
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

  int
  gettoken(char **ps, char *es, char **q, char **eq) {
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
      s++;
      if (*s == '|')
      {
        ret = 'O';
        s++;
      }
      break;
    case '&':
      s++;
      if (*s == '&')
      {
        ret = 'A';
        s++;
      }
      break;
    case '(':
    case ')':
    case ';':
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
      {
        char quote = 0;
        while (s < es && (quote || (!strchr(whitespace, *s) && !strchr(symbols, *s))))
        {
          if (*s == '\\' && quote != '\'' && s + 1 < es)
          {
            s += 2;
            continue;
          }
          if ((*s == '\'' || *s == '"') && (!quote || quote == *s))
          {
            quote = quote ? 0 : *s;
          }
          ++s;
        }
        if (quote)
        {
          panic("unterminated quote");
        }
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

  int
  peek(char **ps, char *es, char *toks) {
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
  struct cmd *parseandor(char **, char *);
  struct cmd *parseexec(char **, char *);
  struct cmd *nulterminate(struct cmd *);

  struct cmd *
  parsecmd(char *s) {
    char *es;
    struct cmd *cmd;

    es = s + strlen(s);
    cmd = parseline(&s, es);
    peek(&s, es, "");
    if (s != es)
    {
      ui::error("leftovers: %s\n", s);
      panic("syntax");
    }
    nulterminate(cmd);
    return cmd;
  }

  struct cmd *
  parseline(char **ps, char *es) {
    struct cmd *cmd;

    cmd = parseandor(ps, es);
    while (peek(ps, es, "&") && (*ps)[1] != '&')
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

  struct cmd *
  parseandor(char **ps, char *es) {
    struct cmd *tree = parsepipe(ps, es);
    while (peek(ps, es, "&|") && (*ps)[1] == **ps)
    {
      int token = gettoken(ps, es, nullptr, nullptr);
      tree = listcmd(tree, parsepipe(ps, es));
      tree->type = token == 'A' ? AND : OR;
    }
    return tree;
  }

  struct cmd *
  parsepipe(char **ps, char *es) {
    struct cmd *cmd;

    cmd = parseexec(ps, es);
    if (peek(ps, es, "|") && (*ps)[1] != '|')
    {
      gettoken(ps, es, 0, 0);
      cmd = pipecmd(cmd, parsepipe(ps, es));
    }
    return cmd;
  }

  struct cmd *
  parseredirs(struct cmd *cmd, char **ps, char *es) {
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
        cmd = redircmd(cmd, q, eq, O_WRONLY | O_CREATE | O_APPEND, 1);
        break;
      }
    }
    return cmd;
  }

  struct cmd *
  parseblock(char **ps, char *es) {
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

  struct cmd *
  parseexec(char **ps, char *es) {
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
  struct cmd *
  nulterminate(struct cmd *cmd) {
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

    case AND:
    case OR:
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
