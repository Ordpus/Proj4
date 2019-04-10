#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>

void* phonecall(void* argp);
void change_connected(int connected, int change, sem_t lock);

int main(void) {
  pthread_t callers[20];
  for(int i = 0; i < 20; ++i) {
    pthread_create(&callers[i], NULL, phonecall, (void*) i);
  }
  
  for(int i = 0; i < 20; ++i) {
    pthread_join(callers[i], NULL);
  }
  return 0;
}

void* phonecall(void* argp) {
	long id = (long) argp;
    static int NUM_OPERATORS = 3;
    static int NUM_LINES = 5;
    static int connected = 0;    // Callers that are connected
    static sem_t connected_lock;
    static sem_t operators;
    static int has_init = 0;
    if(!has_init) {
      if(sem_init(&operators, 0, NUM_OPERATORS) == -1) {
        perror("operators");
        exit(-1);
      }
      if(sem_init(&connected_lock, 0, 1) == -1) {
        perror("connected");
        exit(-1);
      }
      has_init = 1;
    }
	sem_wait(&connected_lock);
    if(connected >= NUM_LINES) {
      printf("Thread [%ld] is calling line, busy signal\n", id);
    }
    while(connected >= NUM_LINES);
    printf("Thread [%ld] has available line, call ringing\n", id);
	connected += 1;
    sem_post(&connected_lock);
    sem_wait(&operators);
    printf("Thread [%ld] is speaking to operator\n", id);
    sleep(3);
    printf("Thread [%ld] has bought a ticket!\n", id);
    printf("Thread [%ld] has hung up!\n", id);
	connected -= 1;
    sem_post(&operators);
    pthread_exit(NULL);
    return NULL;
}

void change_connected(int connected, int change, sem_t lock) {
  sem_wait(&lock);
  *&connected += change;
  sem_post(&lock);
}
