#include <common.h>

static Context* do_event(Event e, Context* c) {
  switch (e.event) {
    case EVENT_YIELD: 
      // Log("EVENT_YIELD: cause = %d", c->mcause);
      extern Context *schedule(Context *prev);
      c = schedule(c);
      break;
    case EVENT_SYSCALL:
      // Log("EVENT_SYSCALL: cause = %d", c->mcause);
      extern void do_syscall(Context *c);
      do_syscall(c);
      break;
    default: panic("Unhandled event ID = %d", e.event);
  }
  // Log("c = %p, c->mepc = %p", c, c->mepc);
  return c;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  cte_init(do_event);
}
