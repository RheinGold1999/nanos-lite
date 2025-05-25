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

void context_uload(PCB *p, const char *filename, char *const argv[], char *const envp[]) {
  Context *cxt = ucontext(NULL, (Area){p->stack, p + 1}, NULL);
  extern uintptr_t loader(PCB *pcb, const char *filename);
  cxt->mepc = loader(p, filename);

  extern void *new_page(size_t nr_page);
  char *sp = new_page(8);

  int argc = 0;
  while (argv != NULL && argv[argc] != NULL) {
    argc++;
  }
  char **argv_tmp = NULL;
  if (argc > 0) {
    for (int i = 0; i < argc; ++i) {
      sp -= (strlen(argv[i]) + 1);
      strcpy(sp, argv[i]);
      *(argv_tmp + i) = sp;
    }
  }

  int envp_nr = 0;
  while (envp != NULL && envp[envp_nr] != NULL) {
    envp_nr++;
  }
  char **envp_tmp = NULL;
  if (envp_nr > 0) {
    for (int i = 0; i < envp_nr; ++i) {
      sp -= (strlen(envp[i]) + 1);
      strcpy(sp, envp[i]);
      *(envp_tmp + i) = sp;
    }
  }

  sp -= sizeof(char *);
  memset(sp, 0, sizeof(char *));

  sp -= sizeof(char *) * envp_nr;
  memcpy(sp, envp_tmp, sizeof(char *) * envp_nr);

  sp -= sizeof(char *);
  memset(sp, 0, sizeof(char *));

  sp -= sizeof(char *) * argc;
  memcpy(sp, argv_tmp, sizeof(char *) * argc);

  sp -= sizeof(argc);
  memset(sp, argc, sizeof(argc));

  cxt->GPRx = (uintptr_t)sp;
  p->cp = cxt;
}

void init_proc() {
  context_kload(&pcb[0], hello_fun, (void *)0L);
  // context_kload(&pcb[1], hello_fun, (void *)1L);
  // context_uload(&pcb[0], "/bin/hello");
  context_uload(&pcb[1], "/bin/exec", NULL, NULL);
  switch_boot_pcb();

  yield();
  Log("Initializing processes...");

  // load program here
  // extern void naive_uload(PCB *pcb, const char *filename);
  // naive_uload(NULL, "/bin/menu");

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

