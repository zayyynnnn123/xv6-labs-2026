#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"
#include "kernel/fs.h"

char *
fmtname(char *path)
{
  static char buf[DIRSIZ + 1];
  char *p;

  // Find first character after last slash.
  for (p = path + strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  if (strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf + strlen(p), 0, DIRSIZ - strlen(p));
  return buf;
}

// fork/exec "cmd cmd_argv... file"
void
run_exec(char *cmd, char **cmd_argv, int cmd_argc, char *file)
{
  char *argv[MAXARG];
  int i, pid;

  if (cmd_argc + 2 > MAXARG) {
    fprintf(2, "find: too many arguments\n");
    return;
  }

  for (i = 0; i < cmd_argc; i++)
    argv[i] = cmd_argv[i];
  argv[cmd_argc] = file;
  argv[cmd_argc + 1] = 0;

  pid = fork();
  if (pid < 0) {
    fprintf(2, "find: fork failed\n");
    return;
  }
  if (pid == 0) {
    exec(cmd, argv);
    fprintf(2, "find: exec %s failed\n", cmd);
    exit(1);
  }
  wait(0);
}

void
find(char *path, char *name, char *cmd, char **cmd_argv, int cmd_argc)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if ((fd = open(path, 0)) < 0) {
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }

  if (fstat(fd, &st) < 0) {
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch (st.type) {
  case T_FILE:
    if (strcmp(fmtname(path), name) == 0) {
      if (cmd)
        run_exec(cmd, cmd_argv, cmd_argc, path);
      else
        printf("%s\n", path);
    }
    break;

  case T_DIR:
    if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
      printf("find: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf + strlen(buf);
    *p++ = '/';
    while (read(fd, &de, sizeof(de)) == sizeof(de)) {
      if (de.inum == 0)
        continue;
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      if (stat(buf, &st) < 0) {
        printf("find: cannot stat %s\n", buf);
        continue;
      }
      if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
        continue;
      find(buf, name, cmd, cmd_argv, cmd_argc);
    }
    break;
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  char *path, *name, *cmd = 0;
  char *cmd_argv[MAXARG];
  int cmd_argc = 0;
  int i;

  if (argc < 3) {
    fprintf(2, "usage: find path name [-exec cmd args...]\n");
    exit(1);
  }

  path = argv[1];
  name = argv[2];

  if (argc > 3) {
    if (strcmp(argv[3], "-exec") != 0) {
      fprintf(2, "usage: find path name [-exec cmd args...]\n");
      exit(1);
    }
    if (argc < 5) {
      fprintf(2, "find: -exec requires a command\n");
      exit(1);
    }
    cmd = argv[4];
    for (i = 4; i < argc && cmd_argc < MAXARG - 2; i++)
      cmd_argv[cmd_argc++] = argv[i];
  }

  find(path, name, cmd, cmd_argv, cmd_argc);
  exit(0);
}