#define _DEFAULT_SOURCE
#include <elf.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

void exit_usage(char *cmd) {
  fprintf(stderr, "usage: %s instructions_file out_path\n", cmd);
  exit(EXIT_FAILURE);
}

void exit_perror(char *s) {
  perror(s);
  exit(EXIT_FAILURE);
}

void get_args(int argc, char *argv[], char **inst_path, char **out_path) {
  if (argc != 3)
    exit_usage(argv[0]);
  *inst_path = argv[0];
  *out_path = argv[1];
}

#define TMP_ELF_SIZE (1 << 20)

Elf64_Ehdr *map_elf(char *out_path) {
  auto out_fd = open(out_path, O_RDWR | O_CREAT | O_TRUNC, 0755);
  if (out_fd == -1)
    exit_perror("open");

  // TODO: truncate after file size is known
  // if (ftruncate(out_fd, ELF_SIZE) == -1)
  //   perror("ftrucnate");

  Elf64_Ehdr *elf =
      mmap(NULL, TMP_ELF_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, out_fd, 0);
  if (elf == (void *)-1)
    exit_perror("mmap");

  if (close(out_fd) == -1)
    exit_perror("close");

  return elf;
}

char *map_inst_file(char *inst_path, size_t *n) {
  auto text_fd = open(inst_path, O_RDONLY);
  if (text_fd == -1)
    exit_perror("open");

  // TODO: stat for size
  auto stat_size = 0;
  *n = stat_size;

  char *text = mmap(NULL, stat_size, PROT_READ, MAP_PRIVATE, text_fd, 0);
  if (text == (void *)-1)
    exit_perror("mmap");

  if (close(text_fd) == -1)
    exit_perror("close");

  return text;
}

int main(int argc, char *argv[]) {

  // handle args

  char *inst_path;
  char *out_path;
  get_args(argc, argv, &inst_path, &out_path);

  // setup outfile for writing

  auto elf = map_elf(out_path);
  auto elf_raw = (char *)elf;
  size_t inst_size;
  auto text = map_inst_file(inst_path, &inst_size);

  // write elf header

  printf("writing elf header into 0-%ld\n", sizeof(*elf));

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
  // TODO: Fill in the entry point once the location of the text segment in
  //       VM is known
  // elf->e_entry = ENTRY_POINT;

  elf->e_phoff = sizeof(*elf);
  elf->e_ehsize = sizeof(*elf);
  elf->e_phentsize = sizeof(Elf64_Phdr);
  elf->e_phnum = 1;

  // write program header

  auto phdr = (Elf64_Phdr *)(elf_raw + elf->e_phoff);

  printf("writing program header into %ld-%ld\n", elf->e_phoff,
         sizeof(Elf64_Phdr));

  phdr->p_type = PT_LOAD;
  phdr->p_flags = PF_R | PF_X;
  // TODO: Fill in the text offset once the location of the text segment in
  //       the file is known
  // phdr->p_offset = TEXT_OFFSET;
  // TODO: Fill in the entry point once the location of the text segment in
  //       VM is known
  // phdr->p_vaddr = ENTRY_POINT;
  // phdr->p_paddr = phdr->p_vaddr;
  // TODO: Discover instruction payload size at runtime
  // phdr->p_filesz = TEXT_SIZE;
  // phdr->p_memsz = phdr->p_filesz;

  // write instructions

  // TODO: write instructions

  // printf("writing instructions into %ld-%ld\n", TEXT_OFFSET,
  //        TEXT_OFFSET + TEXT_SIZE);

  // memcpy(elf_raw + TEXT_OFFSET, text, TEXT_SIZE);

  return EXIT_SUCCESS;
}
