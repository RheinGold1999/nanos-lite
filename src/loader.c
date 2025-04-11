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

  ramdisk_read((void *)0x83000000, 0, 0x06dc4);
  ramdisk_read((void *)0x83007000, 0x00898, 0x00898);

  memset((void *)(0x83007000 + 0x00898), 0, (0x008d4 - 0x00898));
  return (uintptr_t) 0x83004e6c;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

