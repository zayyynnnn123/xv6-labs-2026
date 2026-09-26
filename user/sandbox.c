#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/syscall.h"
#include "user/user.h"

void
usage(char *s)
{
  fprintf(2, "Usage: %s <mask> <path> <command>\n", s);
  exit(1);
}

// Sandbox a command by disallowing system calls in mask and
// system calls that are using path
int
main(int argc, char *argv[])
{
  int i;
  int n = 2;
  int mask = 1;
  char *nargv[MAXARG];

  if (argc < 4) {
    usage(argv[0]);
  }

  if (argv[mask][0] < '0' || argv[mask][0] > '9') {
    usage(argv[0]);
  }

  n += 1; // skip path

  // strip off the first n arguments to sandbox
  for (i = n; i < argc && i < MAXARG; i++) {
    nargv[i - n] = argv[i];
  }
  nargv[argc - n] = 0;

  int pid = fork();
  if (pid < 0) {
    printf("%s: exec fork failed\n", argv[0]);
    exit(1);
  }
  if (pid == 0) {
    if (interpose(atoi(argv[mask]), argv[mask + 1]) < 0) {
      printf("%s: interpose failed", argv[0]);
      exit(1);
    }
    exec(nargv[0], nargv);
    printf("%s: exec %s failed\n", argv[0], nargv[0]);
    exit(1);
  } else {
    wait(0);
  }

  return 0;
}
