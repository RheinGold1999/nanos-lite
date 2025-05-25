#include <proc.h>
#include <elf.h>
#include <fs.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

extern int fs_open(const char *pathname, int flags, int mode);
extern size_t fs_read(int fd, void *buf, size_t len);
extern int fs_close(int fd);

uintptr_t loader(PCB *pcb, const char *filename) {
  // TODO();

  // extern size_t ramdisk_read(void *buf, size_t offset, size_t len);

  // Elf_Ehdr ehdr;
  // ramdisk_read(&ehdr, 0, sizeof(ehdr));
  // // check elf magic number
  // assert(*((uint32_t *)ehdr.e_ident) == 0x464c457f);

  // Elf_Phdr phdr[ehdr.e_phnum];
  // ramdisk_read(&phdr, ehdr.e_phoff, sizeof(phdr)*ehdr.e_phnum);
  // for (size_t i = 0; i < ehdr.e_phnum; ++i) {
  //   if (phdr[i].p_type == PT_LOAD) {
  //     ramdisk_read((void *)phdr[i].p_vaddr, phdr[i].p_offset, phdr[i].p_filesz);
  //     memset((void*)(phdr[i].p_vaddr + phdr[i].p_filesz), 0, (phdr[i].p_memsz - phdr[i].p_filesz));
  //   }
  // }

  // return (uintptr_t) ehdr.e_entry;

  int fd = fs_open(filename, 0, 0);
  Elf_Ehdr ehdr;
  fs_read(fd, &ehdr, sizeof(ehdr));
  // const Elf_Ehdr ehdr = ehdr_tmp;
  // check elf magic number
  assert(*((uint32_t *)ehdr.e_ident) == 0x464c457f);
  Log("fd: %d, filename: %s", fd, fs_name(fd));
  Log("entry: %p", ehdr.e_entry);

  Elf_Phdr phdr[ehdr.e_phnum];
  fs_lseek(fd, ehdr.e_phoff, SEEK_SET);
  fs_read(fd, &phdr, sizeof(phdr));
  for (size_t i = 0; i < ehdr.e_phnum; ++i) {
    if (phdr[i].p_type == PT_LOAD) {
      // ramdisk_read((void *)phdr[i].p_vaddr, phdr[i].p_offset, phdr[i].p_filesz);
      Log("LOAD: Offset: %p, VirtAddr: %p, FileSiz: %p, MemSiz: %p", 
        phdr[i].p_offset, phdr[i].p_vaddr, phdr[i].p_filesz, phdr[i].p_memsz);
      fs_lseek(fd, phdr[i].p_offset, SEEK_SET);
      fs_read(fd, (void *)phdr[i].p_vaddr, phdr[i].p_filesz);
      memset((void*)(phdr[i].p_vaddr + phdr[i].p_filesz), 0, (phdr[i].p_memsz - phdr[i].p_filesz));
    }
  }

  return (uintptr_t)ehdr.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
  Log("Finish execute entry()");
}

