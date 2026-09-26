#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/riscv.h"

int
isalnumchar(char c)
{
  return (c >= '0' && c <= '9') ||
         (c >= 'a' && c <= 'z') ||
         (c >= 'A' && c <= 'Z');
}

int
main(int argc, char *argv[])
{
  int npages = 64;
  int sz = npages * PGSIZE;
  char *mem = sbrk(sz);

  if (mem == (char *) -1) {
    printf("sbrk failed\n");
    exit(1);
  }

  for (int i = 0; i < sz; i++) {
    if (isalnumchar(mem[i])) {
      int j = i;
      while (j < sz && isalnumchar(mem[j]))
        j++;
      int len = j - i;
      if (len > 0 && (i == 0 || !isalnumchar(mem[i - 1]))) {
        write(1, mem + i, len);
        write(1, "\n", 1);
      }
      i = j;
    }
  }

  exit(1);
}