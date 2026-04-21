#include <stdio.h>

const long foo = 42;
const long table[] = {1, 2, 3, 4};
const struct Point {
  long x, y;
} p = {1, 2};
static const char *names[] = {"alice", "bob", "carol"};

int main() {
  puts("this string is rodata\n");
  printf("foo: %ld\n", foo);

  for (auto i = 0ul; i < sizeof(table) / sizeof(table[0]); ++i) {
    printf("table[%ld]: %ld\n", i, table[i]);
  }

  printf("p: {%ld, %ld}\n", p.x, p.y);

  for (auto i = 0ul; i < sizeof(names) / sizeof(names[0]); ++i) {
    printf("names[%ld]: %s\n", i, names[i]);
  }

  return 0;
}
