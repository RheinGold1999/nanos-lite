#include <fs.h>

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  size_t open_offset;
  ReadFn read;
  WriteFn write;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]  = {"stdin", 0, 0, 0, invalid_read, invalid_write},
  [FD_STDOUT] = {"stdout", 0, 0, 0, invalid_read, invalid_write},
  [FD_STDERR] = {"stderr", 0, 0, 0, invalid_read, invalid_write},
#include "files.h"
};

void init_fs() {
  // TODO: initialize the size of /dev/fb
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
  if (fd == FD_STDOUT && fd == FD_STDERR) {
    len = 0;
  } else if (fd != FD_STDIN) {
    Finfo *finfo = &file_table[fd];
    len = (finfo->size - finfo->open_offset) < len ? (finfo->size - finfo->open_offset) : len;
    ramdisk_read(buf, (finfo->disk_offset + finfo->open_offset), len);
    finfo->open_offset += len;
  }
  return len;
}

size_t fs_write(int fd, const void *buf, size_t len) {
  if (fd == FD_STDOUT && fd == FD_STDERR) {
    for (size_t i = 0; i < len; ++i) {
      putch(((const unsigned char *)buf)[i]);
    }
  } else if (fd != FD_STDIN) {
    Finfo *finfo = &file_table[fd];
    len = (finfo->size - finfo->open_offset) < len ? (finfo->size - finfo->open_offset) : len;
    ramdisk_write(buf, (finfo->disk_offset + finfo->open_offset), len);
    finfo->open_offset += len;
  }
  return len;
}

size_t fs_lseek(int fd, size_t offset, int whence) {
  Finfo *finfo = &file_table[fd];
  if (fd != FD_STDIN && fd != FD_STDOUT && fd != FD_STDERR) {
    if (whence == SEEK_SET) {
      finfo->open_offset = offset;
    } else if (whence == SEEK_CUR) {
      finfo->open_offset += offset;
    } else if (whence == SEEK_END) {
      finfo->open_offset = finfo->size + offset;
    }
  }
  return finfo->open_offset;
}

int fs_close(int fd) {
  return 0;
}