#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "user/runtime/user.h"
#include "environment.h"

namespace shell
{
int last_status = 0;
bool exit_requested = false;

bool variable(char value, bool first)
{
  return value == '_' || (value >= 'a' && value <= 'z') ||
         (value >= 'A' && value <= 'Z') || (!first && value >= '0' && value <= '9');
}

bool assignment(const char *word)
{
  if (!variable(*word, true))
  {
    return false;
  }
  for (unsigned i = 1; word[i]; ++i)
  {
    if (word[i] == '=')
    {
      return true;
    }
    if (!variable(word[i], false))
    {
      return false;
    }
  }
  return false;
}

char *expand(const char *word)
{
  auto *output = static_cast<char *>(malloc(4096));
  if (!output)
  {
    return nullptr;
  }
  unsigned size = 0;
  char quote = 0;
  while (*word)
  {
    char value = *word++;
    if ((value == '\'' || value == '"') && (!quote || quote == value))
    {
      quote = quote ? 0 : value;
      continue;
    }
    if (value == '\\' && quote != '\'' && *word)
    {
      value = *word++;
    }
    else if (value == '$' && quote != '\'')
    {
      if (*word == '?')
      {
        ++word;
        char digits[12];
        unsigned count = 0, value = last_status;
        do
        {
          digits[count++] = '0' + value % 10;
          value /= 10;
        } while (value);
        while (count)
        {
          if (size == 4095)
          {
            goto fail;
          }
          output[size++] = digits[--count];
        }
        continue;
      }
      bool braces = *word == '{';
      if (braces)
      {
        ++word;
      }
      char name[128];
      unsigned length = 0;
      while (variable(*word, length == 0))
      {
        if (length == sizeof(name) - 1)
        {
          goto fail;
        }
        name[length++] = *word++;
      }
      if (braces)
      {
        if (*word++ != '}')
        {
          goto fail;
        }
      }
      if (!length)
      {
        value = '$';
      }
      else
      {
        name[length] = 0;
        const char *content = getenv(name);
        if (!content)
        {
          content = "";
        }
        while (*content)
        {
          if (size == 4095)
          {
            goto fail;
          }
          output[size++] = *content++;
        }
        continue;
      }
    }
    if (size == 4095)
    {
      goto fail;
    }
    output[size++] = value;
  }
  if (quote)
  {
    goto fail;
  }
  output[size] = 0;
  return output;
fail:
  free(output);
  return nullptr;
}

bool prepare(char **arguments)
{
  unsigned count = 0;
  while (arguments[count])
  {
    char *expanded = expand(arguments[count]);
    if (!expanded)
    {
      while (count)
      {
        free(arguments[--count]);
      }
      arguments[0] = nullptr;
      return false;
    }
    arguments[count++] = expanded;
  }
  unsigned prefix = 0;
  while (prefix < count && assignment(arguments[prefix]))
  {
    char *equal = strchr(arguments[prefix], '=');
    *equal = 0;
    int result = setenv(arguments[prefix], equal + 1, 1);
    free(arguments[prefix++]);
    if (result < 0)
    {
      for (unsigned i = prefix; i < count; ++i)
      {
        free(arguments[i]);
      }
      arguments[0] = nullptr;
      return false;
    }
  }
  for (unsigned i = prefix; i <= count; ++i)
  {
    arguments[i - prefix] = arguments[i];
  }
  return true;
}

int builtin(char **arguments)
{
  if (!arguments[0])
  {
    return 0;
  }
  const char *name = arguments[0];
  if (!strcmp(name, "exit"))
  {
    unsigned value = last_status;
    if (arguments[1])
    {
      if (arguments[2] || !*arguments[1])
      {
        return 2;
      }
      value = 0;
      for (const char *p = arguments[1]; *p; ++p)
      {
        if (*p < '0' || *p > '9' || value > 255 / 10)
        {
          return 2;
        }
        value = value * 10 + *p - '0';
      }
      if (value > 255)
      {
        return 2;
      }
    }
    exit_requested = true;
    return value;
  }
  if (!strcmp(name, "export"))
  {
    if (!arguments[1])
    {
      for (char **entry = environ; entry && *entry; ++entry)
      {
        printf("%s\n", *entry);
      }
      return 0;
    }
    for (unsigned i = 1; arguments[i]; ++i)
    {
      if (!assignment(arguments[i]))
      {
        return 1;
      }
      char *equal = strchr(arguments[i], '=');
      *equal = 0;
      int result = setenv(arguments[i], equal + 1, 1);
      *equal = '=';
      if (result < 0)
      {
        return 1;
      }
    }
    return 0;
  }
  if (!strcmp(name, "unset"))
  {
    for (unsigned i = 1; arguments[i]; ++i)
    {
      if (unsetenv(arguments[i]) < 0)
      {
        return 1;
      }
    }
    return 0;
  }
  if (!strcmp(name, "cd"))
  {
    const char *path = arguments[1] ? arguments[1] : getenv("HOME");
    if (!path || arguments[2] || chdir(path) < 0)
    {
      fprintf(2, "cannot cd %s\n", path ? path : "");
      return 1;
    }
    return 0;
  }
  if (!strcmp(name, "umask"))
  {
    if (!arguments[1])
    {
      unsigned mask = umask(0);
      umask(mask);
      char output[5] = {'0', char('0' + ((mask >> 6) & 7)),
                        char('0' + ((mask >> 3) & 7)), char('0' + (mask & 7)), 0};
      printf("%s\n", output);
      return 0;
    }
    unsigned mask = 0;
    if (arguments[2])
    {
      return 1;
    }
    for (const char *p = arguments[1]; *p; ++p)
    {
      if (*p < '0' || *p > '7' || mask > 0777 / 8)
      {
        return 1;
      }
      mask = mask * 8 + *p - '0';
    }
    umask(mask);
    return 0;
  }
  return -1;
}

bool parent_candidate(const char *line)
{
  char quote = 0;
  for (const char *p = line; *p; ++p)
  {
    if (*p == '\\' && quote != '\'' && p[1])
    {
      ++p;
      continue;
    }
    if ((*p == '\'' || *p == '"') && (!quote || quote == *p))
    {
      quote = quote ? 0 : *p;
      continue;
    }
    if (!quote && strchr("|&;()<>", *p))
    {
      return false;
    }
  }
  if (quote)
  {
    return false;
  }
  unsigned length = strlen(line);
  return (length >= 6 && !memcmp(line, "export", 6)) ||
         (length >= 5 && !memcmp(line, "unset", 5)) ||
         (length >= 5 && !memcmp(line, "umask", 5)) ||
         (line[0] == 'c' && line[1] == 'd' &&
          (line[2] == 0 || line[2] == ' ' || line[2] == '\n')) ||
         strchr(line, '=');
}
} // namespace shell
