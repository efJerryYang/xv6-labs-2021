#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>

static int nthread = 1;
static int round = 0;

struct barrier {
  pthread_mutex_t barrier_mutex;
  pthread_cond_t barrier_cond;
  int nthread;      // Number of threads that have reached this round of the barrier
  int round;     // Barrier round
} bstate;

static void
barrier_init(void)
{
  assert(pthread_mutex_init(&bstate.barrier_mutex, NULL) == 0);
  assert(pthread_cond_init(&bstate.barrier_cond, NULL) == 0);
  bstate.nthread = 0;
}

static void 
barrier()
{
  // YOUR CODE HERE
  //
  // Block until all threads have called barrier() and
  // then increment bstate.round.
  //
  // printf("==================== Entering barrier() ====================\n");
  pthread_mutex_lock(&bstate.barrier_mutex);
  printf("nthread = %d, round = %d\n", bstate.nthread, bstate.round);
  bstate.nthread++;
  while(bstate.nthread != nthread && bstate.nthread != 0){
    int current_round = bstate.round;
    // printf("Entering cond_wait:\n");
    // printf("  nthread = %d, round = %d\n", bstate.nthread, bstate.round);

    pthread_cond_wait(&bstate.barrier_cond, &bstate.barrier_mutex);

    // printf("Exiting cond_wait:\n");
    // printf("  nthread = %d, round = %d\n", bstate.nthread, bstate.round);
    if (current_round != bstate.round) {
      // printf("  round changed!\n");
      break;
    }
  }
  // printf("Exited while loop:\n");
  // printf("  nthread = %d, round = %d\n", bstate.nthread, bstate.round);
  if (bstate.nthread == nthread) {
    // printf("Broadcasting...\n");
    pthread_cond_broadcast(&bstate.barrier_cond);
    bstate.nthread = 0;
    bstate.round++;
    // printf("Broadcasted!\n");
  }
  // printf("==================== Exiting barrier() =====================\n");
  pthread_mutex_unlock(&bstate.barrier_mutex);
}

static void *
thread(void *xa)
{
  long n = (long) xa;
  long delay;
  int i;

  for (i = 0; i < 20000; i++) {
    int t = bstate.round;
    assert (i == t);
    barrier();
    usleep(random() % 100);
  }

  return 0;
}

int
main(int argc, char *argv[])
{
  pthread_t *tha;
  void *value;
  long i;
  double t1, t0;

  if (argc < 2) {
    fprintf(stderr, "%s: %s nthread\n", argv[0], argv[0]);
    exit(-1);
  }
  nthread = atoi(argv[1]);
  tha = malloc(sizeof(pthread_t) * nthread);
  srandom(0);

  barrier_init();

  for(i = 0; i < nthread; i++) {
    assert(pthread_create(&tha[i], NULL, thread, (void *) i) == 0);
  }
  for(i = 0; i < nthread; i++) {
    assert(pthread_join(tha[i], &value) == 0);
  }
  printf("OK; passed\n");
}
/**
 * Analysis of a deadlock in the barrier() function.
 * 
 * The deadlock occurs when the last thread to reach the barrier is the last thread to exit the while loop.
 *
 * Terminal output:
    ==================== Entering barrier() ====================  // Thread 0 enters barrier
    nthread = 0, round = 0                                        // Thread 0 acquires lock
    Entering cond_wait:                                           // Thread 0 enters cond_wait
      nthread = 1, round = 0                                      // Thread 0 releases lock and waits
    ==================== Entering barrier() ====================  // Thread 1 enters barrier
    nthread = 1, round = 0                                        // Thread 1 acquires lock
    Exited while loop:                                            // Thread 1 skips cond_wait
      nthread = 2, round = 0                                      
    Broadcasting...                                               // Thread 1 broadcasts
    Broadcasted!
    ==================== Exiting barrier() =====================  // Thread 1 exits barrier, releases lock
    Exiting cond_wait:                                            // Thread 0 wakes up, acquires lock
      nthread = 0, round = 1
    Exited while loop:                                            // Thread 0 satisfies while loop condition and exits
      nthread = 0, round = 1
    ==================== Exiting barrier() =====================  // Thread 0 exits barrier, releases lock
    ==================== Entering barrier() ====================  // Thread 0 enters barrier
    nthread = 0, round = 1                                        // Thread 0 acquires lock
    Entering cond_wait:                                           // Thread 0 enters cond_wait
      nthread = 1, round = 1                                      // Thread 0 releases lock and waits
    ==================== Entering barrier() ====================  // Thread 1 enters barrier
    nthread = 1, round = 1                                        // Thread 1 acquires lock
    Exited while loop:                                            // Thread 1 skips cond_wait
      nthread = 2, round = 1
    Broadcasting...                                               // Thread 1 broadcasts
    Broadcasted!
    ==================== Exiting barrier() =====================  // Thread 1 exits barrier, releases lock
    ==================== Entering barrier() ====================  // Thread 1 enters barrier
    nthread = 0, round = 2                                        // Thread 1 acquires lock
    Entering cond_wait:                                           // Thread 1 enters cond_wait
      nthread = 1, round = 2
    Exiting cond_wait:                                            // Bugs here, if a thread broadcasted other threads and then raced in the next round 
      nthread = 1, round = 2                                      // and reached the barrier before the last thread exited the while loop, the last thread
    Entering cond_wait:                                           // would be stuck in the cond_wait.
      nthread = 1, round = 2
 * 
 * 
 * 
 */