#include <common.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
# define MULTIPROGRAM_YIELD() yield()
#else
# define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [AM_KEY_NONE] = "NONE",
  AM_KEYS(NAME)
};

size_t serial_write(const void *buf, size_t offset, size_t len) {
  for (size_t i = 0; i < len; ++i) {
    putch(((const char *)buf)[i]);
  }
  return len;
}

size_t events_read(void *buf, size_t offset, size_t len) {
  size_t ret_len = 0;
  AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
  if (ev.keycode != AM_KEY_NONE) {
    memcpy(buf, (ev.keydown ? "kd " : "ku "), 3);
    ret_len += 3;

    size_t kbd_name_len = strlen(keyname[ev.keycode]);
    memcpy((buf + ret_len), keyname[ev.keycode], kbd_name_len);
    ret_len += kbd_name_len;

    memcpy((buf + ret_len), "\0", 1);
    ret_len += 1;
    Log("%s", keyname[ev.keycode]);
  }
  return ret_len;
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  AM_GPU_CONFIG_T cfg = io_read(AM_GPU_CONFIG);
  sprintf(buf, "WIDTH:%d\nHEIGHT:%d", cfg.width, cfg.height);
  return strlen(buf);
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  assert(offset % 4 == 0);
  assert(len % 4 == 0);
  AM_GPU_CONFIG_T cfg = io_read(AM_GPU_CONFIG);
  int x = (offset / 4) % cfg.width;
  int y = (offset / 4) / cfg.width;
  Log("x = %d, y = %d, buf = %p, len = %d", x, y, buf, len);
  io_write(AM_GPU_FBDRAW, x, y, (void *)buf, (len / 4), 1, true);
  return len;
}

size_t am_ioe_read(void *buf, size_t offset, size_t len) {
  ioe_read(offset, buf);
  return 0;
}

size_t am_ioe_write(const void *buf, size_t offset, size_t len) {
  ioe_write(offset, (void *)buf);
  return 0;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
