#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

void memdump(char *fmt, char *data, int len);

int
main(int argc, char *argv[])
{
  if (argc == 1) {
    printf("Example 1:\n");
    int a[2] = {61810, 2026};
    memdump("ii", (char *)a, sizeof(a));

    printf("Example 2:\n");
    memdump("S", "a string", sizeof("a string"));

    printf("Example 3:\n");
    char *s = "another";
    memdump("s", (char *)&s, sizeof(s));

    struct sss {
      char *ptr;
      int num1;
      short num2;
      char byte;
      char bytes[8];
    } example;

    example.ptr = "hello";
    example.num1 = 1819438967;
    example.num2 = 100;
    example.byte = 'z';
    strcpy(example.bytes, "xyzzy");

    printf("Example 4:\n");
    memdump("pihcS", (char *)&example, sizeof(example));

    printf("Example 5:\n");
    memdump("sccccc", (char *)&example, sizeof(example));
  } else if (argc == 2) {
    // format in argv[1], up to 512 bytes of data from standard input.
    char data[512];
    int n = 0;
    memset(data, '\0', sizeof(data));
    while (n < sizeof(data)) {
      int nn = read(0, data + n, sizeof(data) - n);
      if (nn <= 0)
        break;
      n += nn;
    }
    memdump(argv[1], data, n);
  } else {
    printf("Usage: memdump [format]\n");
    exit(1);
  }
  exit(0);
}

void
memdump(char *fmt, char *data, int len)
{
  int off = 0;  // how many bytes of `data` we've consumed so far

  for (int fi = 0; fmt[fi] != '\0'; fi++) {
    char f = fmt[fi];

    if (f == 'S') {
      // rest of data: print bytes up to null terminator or end of len
      int i = off;
      while (i < len && data[i] != '\0')
        i++;
      write(1, data + off, i - off);
      write(1, "\n", 1);
      off = len;
      continue;
    }

    int need;
    switch (f) {
    case 'i': need = 4; break;
    case 'p': need = 8; break;
    case 'h': need = 2; break;
    case 'c': need = 1; break;
    case 's': need = 8; break;
    default:  continue;  // unrecognized format char, skip it
    }

    if (off + need > len) {
      printf("memdump: not enough data for '%c'\n", f);
      return;
    }

    switch (f) {
    case 'i': {
      int v;
      memmove(&v, data + off, 4);
      printf("%d\n", v);
      break;
    }
    case 'p': {
      uint64 v;
      memmove(&v, data + off, 8);
      printf("%p\n", (void*)v);
      break;
    }
    case 'h': {
      short v;
      memmove(&v, data + off, 2);
      printf("%d\n", v);
      break;
    }
    case 'c': {
      printf("%c\n", data[off]);
      break;
    }
    case 's': {
      uint64 ptr;
      memmove(&ptr, data + off, 8);
      printf("%s\n", (char*)ptr);
      break;
    }
    }

    off += need;
  }
}
