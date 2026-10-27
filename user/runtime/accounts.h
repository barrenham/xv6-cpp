#pragma once
namespace account
{
struct User { char name[14]; unsigned uid, gid; char home[64]; char shell[32]; };
bool numeric(const char *text, unsigned &value);
bool find(const char *name, User &user);
bool find(unsigned uid, User &user);
bool initialize();
int add(const char *name);
int password_state();
bool verify(const char *name, const char *password);
bool set_password(const char *name, const char *password);
bool read_password(const char *prompt, char *output, unsigned capacity);
bool change_password(const char *name);
[[noreturn]] void enter(const User &user);
}
