#define _GNU_SOURCE
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/fs.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define PART_SIZE (1024 * 1024 * 1024)
#define ACCESS_SIZE (64 * 1024 * 1024)

int main(int argc, char const *argv[]) {
  const char *program = argv[0];

  if (argc != 6) {
    fprintf(stderr,
            "Usage: %s <filename> <kernel's help> <r/w> <acces pattern> <block "
            "size[KB]>\n",
            program);
    exit(EXIT_FAILURE);
  }

  const char *filename = argv[1];

  bool arg_help;
  if (strcmp(argv[2], "on") == 0) {
    arg_help = true;
  } else if (strcmp(argv[2], "off") == 0) {
    arg_help = false;
  } else {
    fprintf(stderr, "<kernel's help> should be 'on' or 'off': %s\n", argv[2]);
    exit(EXIT_FAILURE);
  }

  bool arg_write;
  if (strcmp(argv[3], "r") == 0) {
    arg_write = false;
  } else if (strcmp(argv[3], "w") == 0) {
    arg_write = true;
  } else {
    fprintf(stderr, "<r/w> should be 'r' or 'w': %s\n", argv[3]);
    exit(EXIT_FAILURE);
  }

  bool arg_random;
  if (strcmp(argv[4], "seq") == 0) {
    arg_random = false;
  } else if (strcmp(argv[4], "rand") == 0) {
    arg_random = true;
  } else {
    fprintf(stderr, "<access pattern> should be 'seq' or 'rand': %s\n",
            argv[4]);
    exit(EXIT_FAILURE);
  }

  int part_size = PART_SIZE;
  int access_size = ACCESS_SIZE;
  int block_size = atoi(argv[5]) * 1024;
  if (block_size == 0) {
    fprintf(stderr, "<block size> should be > 0: %s\n", argv[5]);
    exit(EXIT_FAILURE);
  }
  if (access_size % block_size != 0) {
    fprintf(stderr, "access size(%d) should be multiple of <block size>: %s\n",
            access_size, argv[5]);
    exit(EXIT_FAILURE);
  }
  int maxcount = part_size / block_size;
  int count = access_size / block_size;

  int *offset = malloc(maxcount * sizeof(int));
  if (offset == NULL) {
    err(EXIT_FAILURE, "malloc(3)");
  }

  int flags = O_RDWR | O_EXCL;
  if (!arg_help) {
    flags |= O_DIRECT;
  }

  int fd = open(filename, flags);
  if (fd == -1) {
    err(EXIT_FAILURE, "open(2)");
  }

  for (int i = 0; i < maxcount; i++) {
    offset[i] = i;
  }
  if (arg_random) {
    for (int i = maxcount - 1; i > 0; i--) {
      int j = rand() % (i + 1);
      int temp = offset[j];
      offset[j] = offset[i];
      offset[i] = temp;
    }
  }

  int sector_size;
  if (ioctl(fd, BLKSSZGET, &sector_size) == -1) {
    err(EXIT_FAILURE, "ioctl(2)");
  }

  char *buf;
  int e = posix_memalign((void **)&buf, sector_size, block_size);
  if (e) {
    errno = e;
    err(EXIT_FAILURE, "posix_memalign(3)");
  }

  for (int i = 0; i < count; i++) {
    if (lseek(fd, offset[i] * block_size, SEEK_SET) == -1) {
      err(EXIT_FAILURE, "lseek(2)");
    }
    if (arg_write) {
      ssize_t ret = write(fd, buf, block_size);
      if (ret == -1) {
        err(EXIT_FAILURE, "write(2)");
      }
    } else {
      ssize_t ret = read(fd, buf, block_size);
      if (ret == -1) {
        err(EXIT_FAILURE, "read(2)");
      }
    }
  }
  if (fdatasync(fd) == -1) {
    err(EXIT_FAILURE, "fdatasync(2)");
  }
  if (close(fd) == -1) {
    err(EXIT_FAILURE, "close(2)");
  }
}
