#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void * printHello(void* arg) {
    int tid;
    tid = (long)arg;
    printf("Hello World %d\n",tid);
}

int main() {
#define NTHREADS 20
    pthread_t thread[NTHREADS];

    int tid = 0;
    pthread_create(&thread[0], NULL, printHello, (void*)tid);
    
}
