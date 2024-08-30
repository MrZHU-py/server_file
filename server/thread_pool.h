#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <pthread.h>
#include "task_queue.h"

extern TaskQueue taskQueue;
extern pthread_mutex_t queueMutex;
extern pthread_cond_t queueCond;

void *workerThread(void *arg);

#endif // THREAD_POOL_H