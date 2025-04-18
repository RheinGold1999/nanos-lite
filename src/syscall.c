#include <common.h>
#include "syscall.h"
#include "fs.h"

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
      const void *buf = (const void *) c->GPR3;
      size_t count = (size_t) c->GPR4;
      Log("SYS_write: fd=%d, buf=%p, count=%d", fd, buf, count);
      // if (fd == 1 || fd == 2) {
      //   for (size_t i = 0; i < count; ++i) {
      //     putch(buf[i]);
      //   }
      //   c->GPRx = count;
      // } else {
      //   Log("SYS_write: fd not supported yet");
      //   c->GPRx = -1;
      // }
      // assert(0 <= fd && fd < LENGTH(file_table));
      if (file_table[fd].write != NULL) {
        c->GPRx = file_table[fd].write(buf, 0, count);
      } else {
        c->GPRx = fs_write(fd, buf, count);
      }
      c->mepc += 4;
      break;
      
    case SYS_brk:
      void *brk = (void *) c->GPR2;
      Log("SYS_brk: brk=%p", brk);
      c->GPRx = 0;
      c->mepc += 4;
      break;

    case SYS_gettimeofday:
      Log("SYS_gettimeofday");
      uint64_t *tv = (uint64_t *)c->GPR2;
      AM_TIMER_UPTIME_T reg = io_read(AM_TIMER_UPTIME);
      tv[0] = reg.us / 1000000;
      tv[1] = reg.us;
      c->GPRx = 0;
      c->mepc += 4;
      break;

    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
