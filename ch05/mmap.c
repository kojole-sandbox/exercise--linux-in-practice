#define _GNU_SOURCE
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#define ALLOC_SIZE (100 * 1024 * 1024)

int main(void) {
  pid_t pid = getpid();

  char *command;
  if (asprintf(&command, "cat /proc/%d/maps", pid) == -1) {
    err(EXIT_FAILURE, "asprintf(3)");
  }

  puts("*** memory map before memory allocation ***");
  fflush(stdout);
  system(command);

  void *new_memory = mmap(NULL, ALLOC_SIZE, PROT_READ | PROT_WRITE,
                          MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (new_memory == MAP_FAILED) {
    err(EXIT_FAILURE, "mmap(2)");
  }
  puts("");
  printf("*** succeeded to allocate memory: address = %p; size = 0x%x ***\n",
         new_memory, ALLOC_SIZE);
  puts("");

  puts("*** memory map after memory allocation ***");
  fflush(stdout);
  system(command);

  if (munmap(new_memory, ALLOC_SIZE) == -1) {
    err(EXIT_FAILURE, "munmap(2)");
  }
}
