#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

static void child() {
  char *args[] = {"/bin/echo", "hello", NULL};
  printf("I'm child! my pid is %d.\n", getpid());
  fflush(stdout);
  execve("/bin/echo", args, NULL);
  err(EXIT_FAILURE, "execve(2)");
}

static void parent(pid_t pid_c) {
  printf("I'm parent! my pid id %d and the pid of my child is %d.\n", getpid(),
         pid_c);
  exit(EXIT_SUCCESS);
}

int main(void) {
  pid_t ret = fork();
  if (ret == -1) {
    err(EXIT_FAILURE, "fork(2)");
  } else if (ret == 0) {
    child();
  } else {
    parent(ret);
  }

  err(EXIT_FAILURE, "unreachable");
}
