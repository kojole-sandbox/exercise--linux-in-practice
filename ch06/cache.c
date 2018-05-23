#define _GNU_SOURCE
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#define CACHE_LINE_SIZE 64
#define NLOOP (4UL * 1024 * 1024 * 1024)
#define NSECS_PER_SEC 1000000000UL

static inline long long diff_nsec(struct timespec a, struct timespec b) {
  return ((b.tv_sec * NSECS_PER_SEC + b.tv_nsec) -
          (a.tv_sec * NSECS_PER_SEC + a.tv_nsec));
}

int main(int argc, char const *argv[]) {
  const char *program = argv[0];

  if (argc != 2) {
    fprintf(stderr, "Usage: %s <size[KB]>\n", program);
    exit(EXIT_FAILURE);
  }

  register int size = atoi(argv[1]) * 1024;
  if (!size) {
    fprintf(stderr, "size should be >= 1: %d\n", size);
    exit(EXIT_FAILURE);
  }

  char *buffer = mmap(NULL, size, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (buffer == MAP_FAILED) {
    err(EXIT_FAILURE, "mmap(2)");
  }

  struct timespec before;
  clock_gettime(CLOCK_MONOTONIC, &before);

  for (int i = 0; i < NLOOP / (size / CACHE_LINE_SIZE); i++) {
    for (int j = 0; j < size; j += CACHE_LINE_SIZE) {
      buffer[j] = 0;
    }
  }

  struct timespec after;
  clock_gettime(CLOCK_MONOTONIC, &after);

  printf("%f\n", (double)diff_nsec(before, after) / NLOOP);

  if (munmap(buffer, size) == -1) {
    err(EXIT_FAILURE, "munmap(2)");
  }
}
