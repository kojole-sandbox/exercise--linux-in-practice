#define _GNU_SOURCE
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define ALLOC_SIZE (100 * 1024 * 1024)

static char overwrite_data[] = "HELLO";

int main(void) {
  pid_t pid = getpid();
  char *command;
  if (asprintf(&command, "cat /proc/%d/maps", pid) == -1) {
    err(EXIT_FAILURE, "asprintf(3)");
  }

  puts("** memory map before mapping file ***");
  fflush(stdout);
  system(command);

  int fd = open("testfile", O_RDWR);
  if (fd == -1) {
    err(EXIT_FAILURE, "open(2)");
  }

  char *file_contents =
      mmap(NULL, ALLOC_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (file_contents == MAP_FAILED) {
    err(EXIT_FAILURE, "mmap(2)");
  }

  puts("");
  printf("*** succeeded to map file: address = %p; size = 0x%x ***\n",
         file_contents, ALLOC_SIZE);
  puts("");
  puts("*** memory map after mapping file ***");
  fflush(stdout);
  system(command);

  puts("");
  printf("*** file contents before overwrite mapped region: %s", file_contents);

  memcpy(file_contents, overwrite_data, strlen(overwrite_data));

  puts("");
  printf("*** overwritten mapped region with: %s\n", file_contents);

  if (munmap(file_contents, ALLOC_SIZE) == -1) {
    warn("munmap(2)");
  }
  if (close(fd) == -1) {
    warn("close(2)");
  }
}
