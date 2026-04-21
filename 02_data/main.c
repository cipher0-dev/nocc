#include <stdio.h>

long g_val = 12345;

void bar() {
  g_val++;
  printf("g_val: %ld\n", g_val);
}

void foo() {
  static long s_val = 54321;
  s_val++;
  printf("s_val: %ld\n", s_val);
}

int main() {
  foo();
  foo();

  bar();
  bar();

  return 0;
}
