#include <proc.h>
#include <elf.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

static uintptr_t loader(PCB *pcb, const char *filename) {
  // TODO();

  extern size_t ramdisk_read(void *buf, size_t offset, size_t len);

  ramdisk_read((void *)0x83000000, 0, 0x06e60);
  ramdisk_read((void *)0x83007000, 0x007000, 0x008a0);

  memset((void *)(0x83007000 + 0x008a0), 0, (0x008e0 - 0x008a0));
  return (uintptr_t) 0x83004ee8;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

