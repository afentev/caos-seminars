
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>

const size_t NUMTHREADS = 20;

int done = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void* thread_entry(void* id) {
  const int myid = (int)id;
  
  const int workloops = 5;
  for (int i = 0; i < workloops; i++) {
      printf("[thread %d] working (%d/%d)\n", myid, i, workloops);
      sleep(1); // simulate doing some costly work
    }
  
  pthread_mutex_lock(&mutex);
  done++;
  printf("[thread %d] done is now %d. Signalling cond.\n", myid, done);

  // wake up the main thread (if it is sleeping) to test the value of done  
  pthread_cond_signal(&cond); 
  pthread_mutex_unlock(&mutex);

  return NULL;
}

int main( int argc, char** argv ) {
  pthread_t threads[NUMTHREADS];

  for(int t = 0; t < NUMTHREADS; t++) {
    pthread_create(&threads[t], NULL, thread_entry, (void*)(long)t);
  }

  // we're going to test "done" so we need the mutex for safety
  pthread_mutex_lock(&mutex);

  // are the other threads still busy?
  while( done < NUMTHREADS ) {
      printf("[thread main] done is %d which is < %d so waiting on cond\n", done, (int)NUMTHREADS);
      
      /* block this thread until another thread signals cond. While
	 blocked, the mutex is released, then re-aquired before this
	 thread is woken up and the call returns. */ 
      pthread_cond_wait(&cond, &mutex); 
      
      puts( "[thread main] wake - cond was signalled." ); 
      
      /* we go around the loop with the lock held */
    }
  
  printf("[thread main] done == %d so everyone is done\n", (int)NUMTHREADS);
  
  pthread_mutex_unlock(&mutex);
  
  return 0;
}