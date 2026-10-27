#pragma once
namespace account
{
struct User { char name[14]; unsigned uid, gid; char home[64]; char shell[32]; };
bool numeric(const char *text, unsigned &value);
bool find(const char *name, User &user);
bool find(unsigned uid, User &user);
bool initialize();
int add(const char *name);
}
