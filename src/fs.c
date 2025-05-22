#include <fs.h>

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENT, FD_DISP, FD_IOE};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

extern size_t serial_write(const void *buf, size_t offset, size_t len);
extern size_t events_read(void *buf, size_t offset, size_t len);
extern size_t dispinfo_read(void *buf, size_t offset, size_t len);
extern size_t fb_write(const void *buf, size_t offset, size_t len);
extern size_t am_ioe_read(void *buf, size_t offset, size_t len);
extern size_t am_ioe_write(const void *buf, size_t offset, size_t len);

/* This is the information about all files in disk. */
Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]  = {"stdin", 0, 0, 0, invalid_read, invalid_write},
  [FD_STDOUT] = {"stdout", 0, 0, 0, invalid_read, serial_write},
  [FD_STDERR] = {"stderr", 0, 0, 0, invalid_read, serial_write},
  [FD_FB]     = {"/dev/fb", 0, 0, 0, invalid_read, fb_write},
  [FD_EVENT]  = {"/dev/events", 0, 0, 0, events_read, invalid_write},
  [FD_DISP]   = {"/proc/dispinfo", 0, 0, 0, dispinfo_read, invalid_write},
  [FD_IOE]    = {"/dev/am_ioe", 128, 0, 0, am_ioe_read, am_ioe_write},
#include "files.h"
};

void init_fs() {
  // TODO: initialize the size of /dev/fb
  AM_GPU_CONFIG_T cfg = io_read(AM_GPU_CONFIG);
  file_table[FD_FB].size = cfg.width * cfg.height * 4;
}

extern size_t ramdisk_read(void *buf, size_t offset, size_t len);
extern size_t ramdisk_write(const void *buf, size_t offset, size_t len);

const char *fs_name(int fd) {
  assert(0 <= fd && fd < LENGTH(file_table));
  return file_table[fd].name;
}

int fs_open(const char *pathname, int flags, int mode) {
  for (int i = 0; i < LENGTH(file_table); ++i) {
    if (strcmp(pathname, file_table[i].name) == 0) {
      file_table[i].open_offset = 0;
      return i;
    }
  }
  panic("pathname is invalid: %s\n", pathname);
}

size_t fs_read(int fd, void *buf, size_t len) {
  // if (fd == FD_STDOUT && fd == FD_STDERR) {
  //   len = 0;
  // } else if (fd != FD_STDIN) {
  Finfo *finfo = &file_table[fd];
  len = (finfo->size - finfo->open_offset) < len ? (finfo->size - finfo->open_offset) : len;
  ramdisk_read(buf, (finfo->disk_offset + finfo->open_offset), len);
  finfo->open_offset += len;
  // }
  return len;
}

size_t fs_write(int fd, const void *buf, size_t len) {
  // if (fd == FD_STDOUT && fd == FD_STDERR) {
  //   for (size_t i = 0; i < len; ++i) {
  //     putch(((const unsigned char *)buf)[i]);
  //   }
  // } else if (fd != FD_STDIN) {
  Finfo *finfo = &file_table[fd];
  len = (finfo->size - finfo->open_offset) < len ? (finfo->size - finfo->open_offset) : len;
  ramdisk_write(buf, (finfo->disk_offset + finfo->open_offset), len);
  finfo->open_offset += len;
  // }
  return len;
}

size_t fs_lseek(int fd, size_t offset, int whence) {
  Finfo *finfo = &file_table[fd];
  assert(0 <= fd && fd < LENGTH(file_table));
  // if (fd != FD_STDIN && fd != FD_STDOUT && fd != FD_STDERR) {
  if (whence == SEEK_SET) {
    finfo->open_offset = offset;
  } else if (whence == SEEK_CUR) {
    finfo->open_offset += offset;
  } else if (whence == SEEK_END) {
    finfo->open_offset = finfo->size + offset;
  }
  // }
  return finfo->open_offset;
}

int fs_close(int fd) {
  file_table[fd].open_offset = 0;
  return 0;
}