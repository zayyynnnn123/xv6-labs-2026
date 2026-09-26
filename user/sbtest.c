#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

static void
report(char *what, int fd)
{
  if (fd < 0) {
    printf("sbtest: forkpath child %s denied\n", what);
  } else {
    printf("sbtest: forkpath child %s ok\n", what);
    close(fd);
  }
}

int
main(int argc, char *argv[])
{
  int r;

  if (argc != 2) {
    fprintf(2, "Usage: sbtest getpid|fork|escape|forkpath\n");
    exit(1);
  }

  // getpid and fork take no argument, so a kernel that blocks a call by
  // corrupting its arguments cannot make them fail.
  if (strcmp(argv[1], "getpid") == 0) {
    printf("sbtest: getpid ret %d\n", getpid());
  } else if (strcmp(argv[1], "fork") == 0) {
    r = fork();
    if (r == 0) {
      printf("sbtest: CHILD RAN\n");
      exit(0);
    }
    if (r > 0)
      wait(0);
    printf("sbtest: fork ret %d\n", r);
  } else if (strcmp(argv[1], "escape") == 0) {
    interpose(0, 0);
    interpose(0, "README");
    r = open("README", O_RDONLY);
    if (r < 0) {
      printf("sbtest: escape confined\n");
    } else {
      printf("sbtest: escape ESCAPED\n");
      close(r);
    }
  } else if (strcmp(argv[1], "forkpath") == 0) {
    r = fork();
    if (r == 0) {
      report("README", open("README", O_RDONLY));
      report("exec.sh", open("exec.sh", O_RDONLY));
      exit(0);
    }
    if (r > 0)
      wait(0);
  } else {
    fprintf(2, "sbtest: unknown test %s\n", argv[1]);
    exit(1);
  }

  printf("sbtest: alive\n");
  exit(0);
}
