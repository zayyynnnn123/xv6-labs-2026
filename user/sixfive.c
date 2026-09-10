#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

char *seps = " -\r\t\n./,";

int
issep(char c)
{
  return strchr(seps, c) != 0;
}

#define MAXTOK 128

void
process_token(char *tok, int len)
{
  if (len == 0)
    return;

  int value = 0;
  for (int i = 0; i < len; i++) {
    if (tok[i] < '0' || tok[i] > '9')
      return;  // token has a non-digit char -> not a pure number, ignore it
    value = value * 10 + (tok[i] - '0');
  }

  if (value % 5 == 0 || value % 6 == 0)
    printf("%d\n", value);
}

void
sixfive(int fd)
{
  char c;
  char tok[MAXTOK];
  int tlen = 0;

  while (read(fd, &c, 1) == 1) {
    if (issep(c)) {
      process_token(tok, tlen);
      tlen = 0;
    } else {
      if (tlen < MAXTOK - 1)
        tok[tlen++] = c;
    }
  }
  process_token(tok, tlen);  // end of file counts as an implicit separator
}

int
main(int argc, char *argv[])
{
  if (argc <= 1) {
    sixfive(0);  // fd 0 = stdin
    exit(0);
  }

  for (int i = 1; i < argc; i++) {
    int fd = open(argv[i], 0);
    if (fd < 0) {
      printf("sixfive: cannot open %s\n", argv[i]);
      continue;
    }
    sixfive(fd);
    close(fd);
  }
  exit(0);
}