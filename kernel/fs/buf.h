#pragma once
struct buf {
  int valid; // has data been read from disk?
  uint dev;
  uint blockno;
  xv::sync::SleepLock lock;
  uint refcnt;
  struct buf *prev; // LRU cache list
  struct buf *next;
  uchar data[BSIZE];
};
