#include <common.h>
#include "syscall.h"
#include "fs.h"

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  int fd = c->GPR2;
  void *buf = (void *) c->GPR3;
  size_t count = (size_t) c->GPR4;

  switch (a[0]) {
    case SYS_exit:
      Log("SYS_exit: call halt(%d)", c->GPR2); // c->GPR2 stores status for SYS_exit
      halt(c->GPR2);
      c->GPRx = 0;
      break;

    case SYS_yield:
      Log("SYS_yield: call yield()");
      yield();
      c->GPRx = 0;
      break;

    case SYS_open:
      const char *path = (const char *)c->GPR2;
      // Log("SYS_open: fd=%d, file=%s", fd, path);
      c->GPRx = fs_open(path, 0, 0);
      break;

    case SYS_close:
      // Log("SYS_close: fd=%d, file=%s", fd, fs_name(fd));
      fs_close(fd);
      c->GPRx = 0;
      break;

    case SYS_lseek:
      size_t offset = (size_t)c->GPR3;
      int whence = c->GPR4;
      // Log("SYS_lseek: fd=%d, file=%s, offset=%d, whence=%d",fd, fs_name(fd), offset, whence);
      // Log("fd=%d, prev open_offset=%d", fd, file_table[fd].open_offset);
      c->GPRx = fs_lseek(fd, offset, whence);
      // Log("fd=%d, cuur open_offset=%d", fd, file_table[fd].open_offset);
      break;

    case SYS_read:
      // Log("SYS_read: fd=%d, file=%s, buf=%p, count=%d", fd, fs_name(fd), buf, count);
      if (file_table[fd].read != NULL) {
        c->GPRx = file_table[fd].read(buf, 0, count);
      } else {
        c->GPRx = fs_read(fd, buf, count);
      }
      break;

    case SYS_write:
      // Log("SYS_write: fd=%d, file=%s, buf=%p, count=%d", fd, fs_name(fd), buf, count);
      if (file_table[fd].write != NULL) {
        c->GPRx = file_table[fd].write(buf, file_table[fd].open_offset, count);
      } else {
        c->GPRx = fs_write(fd, buf, count);
      }
      break;
      
    case SYS_brk:
      void *brk = (void *) c->GPR2;
      Log("SYS_brk: brk=%p", brk);
      c->GPRx = 0;
      break;

    case SYS_gettimeofday:
      // Log("SYS_gettimeofday");
      uint64_t *tv = (uint64_t *)c->GPR2;
      AM_TIMER_UPTIME_T reg = io_read(AM_TIMER_UPTIME);
      tv[0] = reg.us / 1000000;
      tv[1] = reg.us;
      c->GPRx = 0;
      break;

    case SYS_execve:
      const char *fname = (const char *)c->GPR2;
      Log("SYS_execve: filename=%s", fname);
      // char * const argv[] = (char * const *)c->GPR3;
      // char * const envp[] = (char * const *)c->GPR4;
      extern void naive_uload(void *pcb, const char *filename);
      naive_uload(NULL, fname);
      break;

    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
