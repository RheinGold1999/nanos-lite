#include <common.h>
#include "syscall.h"
void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;

  switch (a[0]) {
    case SYS_exit:
      Log("SYS_exit: call halt(c->GPR2)"); // c->GPR2 stores status for SYS_exit
      halt(c->GPR2);
      c->mepc += 4;
      c->GPRx = 0;
      break;

    case SYS_yield:
      Log("SYS_yield: call yield()");
      yield();
      c->mepc += 4;
      c->GPRx = 0;
      break;

    case SYS_write:
      int fd = c->GPR2;
      unsigned char *buf = (unsigned char *) c->GPR3;
      size_t count = (size_t) c->GPR4;
      Log("SYS_write: fd=%d, buf=%p, count=%d", fd, buf, count);
      if (fd == 1 || fd == 2) {
        for (size_t i = 0; i < count; ++i) {
          putch(buf[i]);
        }
        c->GPRx = count;
      } else {
        Log("SYS_write: fd not supported yet");
        c->GPRx = -1;
      }
      c->mepc += 4;
      break;


    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
