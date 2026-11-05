#pragma once

// Shared POSIX ABI: signed long fields on the RV64 user/kernel boundary.
struct mq_attr
{
  long mq_flags;
  long mq_maxmsg;
  long mq_msgsize;
  long mq_curmsgs;
  long __reserved[4];
};

#define MQ_PRIO_MAX 32768
