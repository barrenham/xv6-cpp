#include "kernel/support/types.h"
#include "kernel/fs/image_paths.h"
#include "kernel/fs/fcntl.h"
#include "kernel/fs/stat.h"
#include "kernel/tty/termios.h"
#include "user/runtime/user.h"
#include "accounts.h"
#include "password_hash.h"

namespace account
{
namespace
{
constexpr unsigned Rounds = 100000;

int line(int fd, char *buffer, unsigned capacity)
{
  unsigned used = 0;
  char c;
  for (;;)
  {
    int n = read(fd, &c, 1);
    if (n < 0)
    {
      return -1;
    }
    if (!n)
    {
      if (!used)
      {
        return 0;
      }
      break;
    }
    if (c == '\n')
    {
      break;
    }
    if (used + 1 >= capacity)
    {
      return -1;
    }
    buffer[used++] = c;
  }
  buffer[used] = 0;
  return 1;
}

bool record(const char *name, char *result)
{
  int fd = open("/etc/shadow", O_RDONLY);
  if (fd < 0)
  {
    return false;
  }
  char buffer[160];
  bool found = false;
  unsigned size = strlen(name);
  while (line(fd, buffer, sizeof(buffer)) > 0)
  {
    if (strlen(buffer) > size && !memcmp(buffer, name, size) && buffer[size] == ':')
    {
      strcpy(result, buffer + size + 1);
      found = true;
      break;
    }
  }
  close(fd);
  return found;
}
} // namespace

int password_state()
{
  struct stat st;
  if (stat("/etc/shadow", &st) < 0)
  {
    return 0;
  }
  if (st.type != T_FILE || st.uid != 0 || (st.mode & 077))
  {
    return -1;
  }
  char data[160];
  return record("root", data) && strlen(data) >= 14 &&
                 !memcmp(data, "pbkdf2-sha256$", 14)
             ? 1
             : -1;
}

bool verify(const char *name, const char *password)
{
  if (geteuid() != 0)
  {
    return false;
  }
  char value[160];
  if (!record(name, value))
  {
    return false;
  }
  const char *prefix = "pbkdf2-sha256$";
  unsigned size = strlen(prefix);
  if (strlen(value) < size || memcmp(value, prefix, size))
  {
    return false;
  }
  char *p = value + size;
  unsigned rounds = 0;
  while (*p >= '0' && *p <= '9')
  {
    if (rounds > 1000000 / 10)
    {
      return false;
    }
    rounds = rounds * 10 + *p++ - '0';
  }
  if (*p++ != '$' || rounds < 10000 || rounds > 1000000 || strlen(p) != 32 + 1 + 64 ||
      p[32] != '$')
  {
    return false;
  }
  unsigned char salt[16], expected[32], actual[32];
  if (!password_hash::unhex(p, 16, salt) || !password_hash::unhex(p + 33, 32, expected))
  {
    return false;
  }
  password_hash::derive(password, strlen(password), salt, rounds, actual);
  unsigned difference = 0;
  for (unsigned i = 0; i < 32; ++i)
  {
    difference |= actual[i] ^ expected[i];
  }
  password_hash::wipe(actual, sizeof(actual));
  password_hash::wipe(value, sizeof(value));
  return difference == 0;
}

bool set_password(const char *name, const char *password)
{
  User user{};
  if (geteuid() != 0 || !find(name, user) || strlen(password) < 8 ||
      strlen(password) > 63 || mkdir("/etc/.shadow.lock") < 0)
  {
    return false;
  }
  unsigned old_mask = umask(0077);
  struct stat previous{};
  bool exists = stat("/etc/shadow", &previous) == 0;
  int input = open("/etc/shadow", O_RDONLY),
      output = open("/etc/.shadow.new", O_CREATE | O_WRONLY | O_TRUNC);
  bool good = output >= 0 && (!exists || (input >= 0 && previous.type == T_FILE &&
                                          previous.uid == 0 && !(previous.mode & 077)));
  if (good)
  {
    good = chown("/etc/.shadow.new", 0, 0) == 0 && chmod("/etc/.shadow.new", 0600) == 0;
  }
  unsigned char salt[16], hash[32];
  // Salts provide uniqueness, not a secret key. The OS has no CSPRNG yet.
  unsigned long ticks = clockticks();
  unsigned long seed[3] = {ticks, static_cast<unsigned long>(getpid()),
                           static_cast<unsigned long>(uptime())};
  password_hash::Sha256 digest;
  digest.add(seed, sizeof(seed));
  digest.add(name, strlen(name));
  char old[160];
  if (input >= 0)
  {
    int state;
    while ((state = line(input, old, sizeof(old))) > 0)
    {
      digest.add(old, strlen(old));
      unsigned n = strlen(name);
      if (strlen(old) > n && !memcmp(old, name, n) && old[n] == ':')
      {
        continue;
      }
      unsigned length = strlen(old);
      old[length++] = '\n';
      if (good && write(output, old, length) != static_cast<int>(length))
      {
        good = false;
      }
    }
    if (state < 0)
    {
      good = false;
    }
    close(input);
  }
  // Include the previous database so password changes do not reuse a salt after reboot.
  digest.finish(hash);
  for (unsigned i = 0; i < 16; ++i)
  {
    salt[i] = hash[i];
  }
  password_hash::derive(password, strlen(password), salt, Rounds, hash);
  char encoded[160], salt_text[33], hash_text[65];
  password_hash::hex(salt, 16, salt_text);
  password_hash::hex(hash, 32, hash_text);
  strcpy(encoded, name);
  strcpy(encoded + strlen(encoded), ":pbkdf2-sha256$100000$");
  strcpy(encoded + strlen(encoded), salt_text);
  strcpy(encoded + strlen(encoded), "$");
  strcpy(encoded + strlen(encoded), hash_text);
  strcpy(encoded + strlen(encoded), "\n");
  if (good &&
      write(output, encoded, strlen(encoded)) != static_cast<int>(strlen(encoded)))
  {
    good = false;
  }
  if (output >= 0)
  {
    close(output);
  }
  if (good)
  {
    good = rename("/etc/.shadow.new", "/etc/shadow") == 0;
  }
  if (!good)
  {
    unlink("/etc/.shadow.new");
  }
  unlink("/etc/.shadow.lock");
  umask(old_mask);
  password_hash::wipe(hash, sizeof(hash));
  password_hash::wipe(hash_text, sizeof(hash_text));
  password_hash::wipe(old, sizeof(old));
  return good;
}

bool read_password(const char *prompt, char *output, unsigned capacity)
{
  termios saved;
  if (tcgetattr(0, &saved) < 0)
  {
    return false;
  }
  termios hidden = saved;
  hidden.c_lflag &= ~(ECHO | ECHONL);
  hidden.c_lflag |= ICANON;
  if (tcsetattr(0, TCSAFLUSH, &hidden) < 0)
  {
    return false;
  }
  write(2, prompt, strlen(prompt));
  unsigned used = 0;
  bool good = true, overflow = false;
  for (;;)
  {
    char c;
    int n = read(0, &c, 1);
    if (n <= 0)
    {
      good = false;
      break;
    }
    if (c == '\n' || c == '\r')
    {
      break;
    }
    if (static_cast<unsigned char>(c) < 32)
    {
      overflow = true;
      continue;
    }
    if (used + 1 < capacity)
    {
      output[used++] = c;
    }
    else
    {
      overflow = true;
    }
  }
  output[used] = 0;
  tcsetattr(0, TCSANOW, &saved);
  write(2, "\n", 1);
  return good && !overflow;
}

bool change_password(const char *name)
{
  char first[64]{}, second[64]{};
  bool good = read_password("New password: ", first, sizeof(first)) &&
              read_password("Retype password: ", second, sizeof(second)) &&
              !strcmp(first, second) && set_password(name, first);
  password_hash::wipe(first, sizeof(first));
  password_hash::wipe(second, sizeof(second));
  if (!good)
  {
    fprintf(
        2,
        "passwd: passwords must match and contain 8..63 bytes; account/database must be writable\n");
  }
  return good;
}

[[noreturn]] void enter(const User &user)
{
  for (int fd = 3; fd < 16; ++fd)
  {
    close(fd);
  }
  clearenv();
  setenv("PATH", XV_DEFAULT_PATH, 1);
  setenv("HOME", user.home, 1);
  setenv("USER", user.name, 1);
  setenv("SHELL", user.shell, 1);
  umask(0022);
  if (setgroups(0, nullptr) < 0 || setgid(user.gid) < 0 || setuid(user.uid) < 0 ||
      chdir(user.home) < 0)
  {
    fprintf(2, "login: cannot establish user session\n");
    exit(1);
  }
  char *arguments[] = {const_cast<char *>(user.shell), nullptr};
  execv(user.shell, arguments);
  fprintf(2, "login: cannot execute shell\n");
  exit(1);
}
} // namespace account
