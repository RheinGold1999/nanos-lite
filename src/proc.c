#include <proc.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite with arg '%d' for the %dth time!", (uintptr_t)arg, j);
    j ++;
    yield();
  }
}

void context_kload(PCB *p, void (*entry)(void *), void *arg) {
  Context *cxt = kcontext((Area){p->stack, p + 1}, entry, arg);
  p->cp = cxt;
}

void init_proc() {
  context_kload(&pcb[0], hello_fun, (void *)0L);
  context_kload(&pcb[1], hello_fun, (void *)1L);
  switch_boot_pcb();

  Log("Initializing processes...");

  // load program here
  extern void naive_uload(PCB *pcb, const char *filename);
  naive_uload(NULL, "/bin/menu");

}

Context* schedule(Context *prev) {
  current->cp = prev;
  if (current == &pcb_boot) {
    current = &pcb[0];
  } else {
    PCB *next;
    do {
      next = (current == &pcb[MAX_NR_PROC - 1]) ? &pcb[0] : current + 1;
      current = next;
    } while (next->cp == NULL);

  }
  return current->cp;
}

