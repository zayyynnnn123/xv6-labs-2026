#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if(argc < 4){
    fprintf(2, "Usage: %s mask path command [args]\n", argv[0]);
    exit(1);
  }

  int pid = fork();
  if(pid < 0){
    fprintf(2, "fork failed\n");
    exit(1);
  }
  if(pid == 0){
    if(interpose(atoi(argv[1]), argv[2]) < 0){
      fprintf(2, "interpose failed\n");
      exit(1);
    }
    exec(argv[3], &argv[3]);
    fprintf(2, "exec %s failed\n", argv[3]);
    exit(1);
  }
  wait(0);
  exit(0);
}
