#ifndef UDP_CLIENT_H
#define UDP_CLIENT_H

#include <pthread.h>

typedef struct {
    double *loads;
    int count;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int data_ready;
    int stop_flag;
} SharedData;

void* udp_worker_thread(void* arg);

#endif