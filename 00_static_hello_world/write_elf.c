#define _DEFAULT_SOURCE
#include <elf.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

void exit_usage(const char *cmd) {
  fprintf(stderr, "usage: %s text_path out_path\n", cmd);
  exit(EXIT_FAILURE);
}

void exit_perror(const char *s) {
  perror(s);
  exit(EXIT_FAILURE);
}

void get_args(int argc, char *argv[], char **text_path, char **out_path) {
  if (argc != 3)
    exit_usage(argv[0]);
  *text_path = argv[1];
  *out_path = argv[2];
}

#define INIT_ELF_SIZE (1 << 20)
#define INIT_TEXT_OFFSET 0x1000
#define VIRT_OFFSET 0x400000
// TODO: calculate these values vs using consts
#define VIRT_ENTRY_ADDR (INIT_TEXT_OFFSET + VIRT_OFFSET + 0x47)
#define FILE_SECTION_HEADERS_OFFSET 8744
#define NUM_PHDRS 7

Elf64_Ehdr *map_elf(char *path) {
  auto fd = open(path, O_RDWR | O_CREAT | O_TRUNC, 0755);
  if (fd == -1)
    exit_perror("open");

  // TODO: truncate again after file size is known
  if (ftruncate(fd, INIT_ELF_SIZE) == -1)
    perror("ftrucnate");

  Elf64_Ehdr *elf =
      mmap(NULL, INIT_ELF_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (elf == (void *)-1)
    exit_perror("mmap elf");

  if (close(fd) == -1)
    exit_perror("close");

  return elf;
}

char *map_text_file(char *path, size_t *n) {
  auto fd = open(path, O_RDONLY);
  if (fd == -1)
    exit_perror("open");

  struct stat statbuf;
  if (fstat(fd, &statbuf) == -1)
    exit_perror("stat");

  *n = statbuf.st_size;

  char *text = mmap(NULL, statbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (text == (void *)-1)
    exit_perror("mmap text");

  if (close(fd) == -1)
    exit_perror("close");

  return text;
}

void write_ehdr(Elf64_Ehdr *elf) {
  elf->e_ident[EI_MAG0] = ELFMAG0;
  elf->e_ident[EI_MAG1] = ELFMAG1;
  elf->e_ident[EI_MAG2] = ELFMAG2;
  elf->e_ident[EI_MAG3] = ELFMAG3;

  elf->e_ident[EI_CLASS] = ELFCLASS64;
  elf->e_ident[EI_DATA] = ELFDATA2LSB;
  elf->e_ident[EI_VERSION] = EV_CURRENT;

  elf->e_type = ET_EXEC;
  elf->e_machine = EM_X86_64;
  elf->e_version = EV_CURRENT;
  elf->e_entry = VIRT_ENTRY_ADDR;

  elf->e_phoff = sizeof(*elf);
  elf->e_shoff = FILE_SECTION_HEADERS_OFFSET;
  elf->e_flags = 0;
  elf->e_ehsize = sizeof(*elf);
  elf->e_phentsize = sizeof(Elf64_Phdr);
  elf->e_phnum = NUM_PHDRS;
  elf->e_shentsize = sizeof(Elf64_Shdr);
  elf->e_shnum = 10;
  elf->e_shstrndx = 9;
}

Elf64_Phdr phdrs[NUM_PHDRS] = {
    {
        .p_type = PT_LOAD,
        .p_flags = PF_R,
        .p_vaddr = VIRT_OFFSET,
        .p_paddr = VIRT_OFFSET,
        // TODO: compute size based off size of build id note
        // .p_filesz = sizeof(Elf64_Ehdr) + sizeof(Elf64_Phdr) * NUM_PHDRS +
        //             sizeof(build_id_note),
        // .p_memsz = sizeof(Elf64_Ehdr) + sizeof(Elf64_Phdr) * NUM_PHDRS +
        //            sizeof(build_id_note),
        .p_filesz = 0x1ec,
        .p_memsz = 0x1ec,
        .p_align = 0x1000,
    },
    {
        .p_type = PT_LOAD,
        .p_flags = PF_R | PF_X,
        .p_offset = INIT_TEXT_OFFSET,
        .p_vaddr = INIT_TEXT_OFFSET + VIRT_OFFSET,
        .p_paddr = INIT_TEXT_OFFSET + VIRT_OFFSET,
        // NOTE: File and mem size filled in at runtime
        // .p_filesz=0,
        // .p_memsz=0,
        .p_align = 0x1000,
    },
    {
        PT_LOAD,
        .p_flags = PF_R,
        .p_align = 0x1000,
    },
    {
        PT_NOTE,
        .p_flags = PF_R,
        .p_align = 0x4,
    },
    {
        PT_NOTE,
        .p_flags = PF_R,
        .p_align = 0x8,
    },
    {
        PT_GNU_PROPERTY,
        .p_flags = PF_R,
        .p_align = 0x8,
    },
    {
        PT_GNU_STACK,
        .p_flags = PF_R | PF_W,
        .p_align = 0x10,
    },
};

void write_phdrs(Elf64_Phdr *phdr) { memcpy(phdr, phdrs, sizeof(phdrs)); }

void write_build_id_note() {}

void write_shdrs() {}

void write_text(char *offset, char *text, size_t n) { memcpy(offset, text, n); }

int main(int argc, char *argv[]) {
  // handle args
  char *text_path;
  char *out_path;
  get_args(argc, argv, &text_path, &out_path);

  // map in files
  auto elf = map_elf(out_path);
  auto elf_raw = (char *)elf;

  size_t text_size;
  auto text = map_text_file(text_path, &text_size);
  phdrs[1].p_filesz = text_size;
  phdrs[1].p_memsz = text_size;

  // write elf header
  write_ehdr(elf);

  // write program headers
  write_phdrs((Elf64_Phdr *)(elf_raw + elf->e_phoff));

  // TODO: write build id note
  write_build_id_note();

  // TODO: write section headers
  write_shdrs();

  // write instructions
  write_text(elf_raw + INIT_TEXT_OFFSET, text, text_size);

  return EXIT_SUCCESS;
}
