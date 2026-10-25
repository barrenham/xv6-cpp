#pragma once

extern "C"
{
void *test_retain_file(void *file);
void test_release_file(void *file);
void *test_retain_inode(void *node);
void test_release_inode(void *node);
[[noreturn]] void test_resource_fail(const char *message);
}
