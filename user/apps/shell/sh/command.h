#pragma once
#include "kernel/support/param.h"

extern "C"
{
// Parsed command representation
#define EXEC 1
#define REDIR 2
#define PIPE 3
#define LIST 4
#define BACK 5
#define AND 6
#define OR 7

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
  int child_status(int child);
  int fork1(void); // Fork but panics on failure.
  void panic(char *);
  struct cmd *parsecmd(char *);
  void runcmd(struct cmd *) __attribute__((noreturn));
}
