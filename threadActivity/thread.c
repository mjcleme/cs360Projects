#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void * printHello(void* arg) {
    int tid;
    tid = (int)arg;
    printf("Hello World %d\n",tid);
    return;
}

int main() {
#define NTHREADS 20
    pthread_t thread[NTHREADS];

    int i = 0;
    for (i = 0; i < NTHREADS; i++) {
        if (i > 0) {
            pthread_join(thread[i-1], NULL);
        }
        if (pthread_create(&thread[i], NULL, printHello, (void*)i) != 0) {
            perror("Unable to create thread");
        }
    }
    
}
