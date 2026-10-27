#pragma once
namespace shell
{
char *expand(const char *word);
bool assignment(const char *word);
bool prepare(char **arguments);
int builtin(char **arguments);
bool parent_candidate(const char *line);
}
