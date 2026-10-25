#pragma once

extern "C"
{
int printk(char *, ...) __attribute__((format(printf, 1, 2)));
[[noreturn]] void panic(char *);
}
