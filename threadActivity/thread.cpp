#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <queue>

//queue socketQueue;

void * printHello(void* arg) {

//  for (;;) {
//      Get socket from the queue
//      read request
//      respond
//  }
    int tid;
    tid = (long)arg;
    printf("Hello World %d\n",tid);
}

int main() {
#define NTHREADS 20
    pthread_t thread[NTHREADS];

    long i = 0;
    for (i = 0; i < NTHREADS; i++) {
//        if (i > 0) {
//            pthread_join(thread[i-1], NULL);
//        }
        if (pthread_create(&thread[i], NULL, printHello, (void*)i) != 0) {
            perror("Unable to create thread");
        }
    }
//  Set up socket, bind, listen
//      for (;;) {
//          fd = accept;
//          put fd in queue;
//          
    pthread_exit(NULL);
}
