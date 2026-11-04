#pragma once
#define F_OK 0
#define X_OK 1
#define W_OK 2
#define R_OK 4
#define S_ISVTX 01000
#define O_RDONLY 0x000
#define O_WRONLY 0x001
#define O_RDWR   0x002
#define O_CREATE 0x200
#define O_TRUNC  0x400

#define O_ACCMODE 0x003
#define O_NONBLOCK 0x800
#define O_APPEND 0x1000
#define O_NOFOLLOW 0x2000
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#define F_GETFL 3
#define F_SETFL 4

#define O_CREAT O_CREATE
#define O_EXCL 0x10000
#define O_CLOEXEC 0x80000
#define FD_CLOEXEC 1
#define F_DUPFD 0
#define F_GETFD 1
#define F_SETFD 2

#define F_DUPFD_CLOEXEC 1030
