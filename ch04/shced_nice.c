#define _DEFAULT_SOURCE

#include <err.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define NLOOP_FOR_ESTIMATION 1000000000UL
#define NSECS_PER_MSEC 1000000UL
#define NSECS_PER_SEC 1000000000UL

static inline long diff_nsec(struct timespec before, struct timespec after) {
  return ((after.tv_sec * NSECS_PER_SEC + after.tv_nsec) -
          (before.tv_sec * NSECS_PER_SEC + before.tv_nsec));
}

static unsigned long loops_per_msec() {
  struct timespec before, after;
  clock_gettime(CLOCK_MONOTONIC, &before);

  for (unsigned long i = 0; i < NLOOP_FOR_ESTIMATION; i++)
    ;

  clock_gettime(CLOCK_MONOTONIC, &after);
  return NLOOP_FOR_ESTIMATION * NSECS_PER_MSEC / diff_nsec(before, after);
}

static inline void load(unsigned long nloop) {
  for (unsigned long i = 0; i < nloop; i++)
    ;
}

static void child_fn(int id, struct timespec *buf, int nrecord,
                     unsigned long nloop_per_resol, struct timespec start) {
  for (int i = 0; i < nrecord; i++) {
    struct timespec ts;
    load(nloop_per_resol);
    clock_gettime(CLOCK_MONOTONIC, &ts);
    buf[i] = ts;
  }
  for (int i = 0; i < nrecord; i++) {
    printf("%d\t%ld\t%d\n", id, diff_nsec(start, buf[i]) / NSECS_PER_MSEC,
           (i + 1) * 100 / nrecord);
  }
  exit(EXIT_SUCCESS);
}

static pid_t *pids;

int main(int argc, char const *argv[]) {
  int ret = EXIT_FAILURE;

  if (argc < 3) {
    fprintf(stderr, "Usage: %s <total[ms]> <resolution[ms]>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  int nproc = 2;
  int total = atoi(argv[1]);
  int resol = atoi(argv[2]);

  if (total < 1) {
    fprintf(stderr, "<total>(%d) should be >= 1\n", total);
    exit(EXIT_FAILURE);
  }
  if (resol < 1) {
    fprintf(stderr, "<resolution>(%d) should be >= 1\n", resol);
    exit(EXIT_FAILURE);
  }
  if (total % resol) {
    fprintf(stderr, "<total>(%d) should be multiple of <resolution[ms]>(%d)\n",
            total, resol);
    exit(EXIT_FAILURE);
  }

  int nrecord = total / resol;
  struct timespec *logbuf = malloc(nrecord * sizeof(struct timespec));
  if (logbuf == NULL) {
    err(EXIT_FAILURE, "malloc(3) logbuf");
  }

  puts("estimating workload which takes just one milisecond");
  unsigned long nloop_per_resol = loops_per_msec() * resol;
  puts("end estimation");

  pids = malloc(nproc * sizeof(pid_t));
  if (pids == NULL) {
    err(EXIT_FAILURE, "malloc(3) pids");
  }

  struct timespec start;
  clock_gettime(CLOCK_MONOTONIC, &start);

  int ncreated = 0;
  bool created_all = false;
  for (int i = 0; i < nproc; i++, ncreated++) {
    pids[i] = fork();
    if (pids[i] == -1) {
      goto wait_children;
    } else if (pids[i] == 0) {
      if (i == 1) {
        nice(5);
      }
      child_fn(i, logbuf, nrecord, nloop_per_resol, start);
    }
  }
  created_all = true;

wait_children:
  if (!created_all) {
    for (int i = 0; i < ncreated; i++) {
      if (kill(pids[i], SIGINT) == -1) {
        warn("kill(2) %d", pids[i]);
      }
    }
  }

  for (int i = 0; i < ncreated; i++) {
    if (wait(NULL) == -1) {
      warn("wait(2)");
    }
  }
}
