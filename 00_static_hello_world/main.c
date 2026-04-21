static inline long sys_write(long fd, const void *buf, unsigned long len) {
  long ret;
  __asm__ volatile("mov $1, %%rax\n"
                   "syscall\n"
                   : "=a"(ret)
                   : "D"(fd), "S"(buf), "d"(len)
                   : "rcx", "r11", "memory");
  return ret;
}

[[noreturn]]
static inline void sys_exit(int code) {
  __asm__ volatile("mov $60, %%rax\n"
                   "syscall\n"
                   :
                   : "D"(code)
                   : "rcx", "r11", "memory");
  __builtin_unreachable();
}
void _start() {
  sys_write(1, "Hello World!\n", 13);
  sys_exit(0);
}
