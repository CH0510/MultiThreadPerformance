/*
 * @Descripttion: 线程屏障同步测试
 * @version: 
 * @ Author: Hao Chen
 * @Date: 2022-02-22 21:04:13
 * @LastEditors: Hao Chen
 * @LastEditTime: 2022-02-22 21:36:37
 */
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/time.h>

int NTHR;  // 线程数量
const long NUMNUM = 8000000L;  // 等待排序的数字数量
long TNUM;  // 每个线程需要排序的数字数量

long nums[NUMNUM];
long snums[NUMNUM];

pthread_barrier_t barrier;

#define heapsort qsort

// conpare two long integer, helper function
int compLong(const void *arg1, const void *arg2) {
  long l1 = *(long *)arg1;
  long l2 = *(long *)arg2;

  if (l1 == l2) return 0;
  else if (l1 < l2) return -1;
  else return 1;
}

// Work thread to sort a portion of the set of numbers.
void *thr_fn(void *arg) {
  long idx = (long) arg;
  heapsort(&nums[idx], TNUM, sizeof(long), compLong);
  pthread_barrier_wait(&barrier);
  
  // Go off and perform more work
  return (void *)0;
}

// Merge the result of the individual sorted range.
void merge() {
  long idx[NTHR];
  long i, minidx, sidx, num;

  for (i = 0; i < NTHR; ++i)
    idx[i] = i * TNUM;
  
  for (sidx = 0; sidx < NUMNUM; sidx++) {
    num = LONG_MAX;
    for (i = 0; i < NTHR; ++i) {
      if ((idx[i] < (i + 1) * TNUM) && (nums[idx[i]] < num)) {
        num = nums[idx[i]];
        minidx = i;
      }
    }

    snums[sidx] = num;
    ++idx[minidx];
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s num_of_thread\n", argv[0]);
    exit(1);
  }

  NTHR = atoi(argv[1]);
  TNUM = NUMNUM / NTHR;

  unsigned long i;
  struct timeval start, end;
  long long startusec, endusec;
  double elapsed;
  int err;
  pthread_t tid;

  // Create the initial set of numbers to sort
  srandom(time(NULL));
  for (i = 0; i < NUMNUM; ++i) {
    nums[i] = random();
  }

  // Create 8 threads to sort the numbers
  gettimeofday(&start, NULL);
  pthread_barrier_init(&barrier, NULL, NTHR + 1);  // 加一是为了主线程也参与，并且负责最后的合并
  for (i = 0; i < (unsigned long)NTHR; ++i) {
    err = pthread_create(&tid, NULL, thr_fn, (void *)(i * TNUM));
    if (err != 0) {
      fprintf(stderr, "pthread_create failed\n");
      exit(1);
    }
  }

  pthread_barrier_wait(&barrier);
  merge();
  gettimeofday(&end, NULL);

  // print the sorted list.
  startusec = start.tv_sec * 1000000 + start.tv_usec;
  endusec = end.tv_sec * 1000000 + end.tv_usec;
  elapsed = (endusec - startusec) / 1000000.0;
  printf("%d threads, sort took %.4f seconds\n", NTHR, elapsed);
  for (i = 0; i < NUMNUM; ++i) {
    printf("%ld\n", snums[i]);
  }

  return 0;
}