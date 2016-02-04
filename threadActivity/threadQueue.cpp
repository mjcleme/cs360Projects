#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <queue>
#include <iostream>
#include <semaphore.h>

sem_t full, empty, mutex;

class myQueue {
    std::queue <int> stlqueue;
    public:
        //Pushes a socket into the queue
        void push(int sock) {
            sem_wait(&empty);
            sem_wait(&mutex);
            stlqueue.push(sock);
            sem_post(&mutex);
            sem_post(&full);
        }
        //Returns a socket and removes it from the queue
        int pop() {
            sem_wait(&full);
            sem_wait(&mutex);
            int val = stlqueue.front();
            stlqueue.pop();
            sem_post(&mutex);
            sem_post(&empty);
            return val;
        }
    
}   sockqueue;


//queue socketQueue;

void * printHello(void* arg) {

    for (;;) {
        std::cout << "Got" << sockqueue.pop() << std::endl;
    }
}

int main() {
#define NTHREADS 10
#define NQUEUE 20
    pthread_t thread[NTHREADS];

    //Initialize semaphores with values
    sem_init(&full, PTHREAD_PROCESS_PRIVATE, 0);
    sem_init(&empty, PTHREAD_PROCESS_PRIVATE, NQUEUE);
    sem_init(&mutex, PTHREAD_PROCESS_PRIVATE, 1);

    long i = 0;
    for (int i = 0; i < 10; i++) {
        sockqueue.push(i);
    }
    for (i = 0; i < NTHREADS; i++) {
        if (pthread_create(&thread[i], NULL, printHello, (void*)i) != 0) {
            perror("Unable to create thread");
        }
    }
    pthread_exit(NULL);
}
