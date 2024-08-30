#include "thread_pool.h"
#include "server.h"
#include <unistd.h>

void *workerThread(void *arg) {
    while (1) {
        int client_fd;

        pthread_mutex_lock(&queueMutex);
        while (taskQueue.empty()) {
            pthread_cond_wait(&queueCond, &queueMutex);
        }
        client_fd = taskQueue.front();
        taskQueue.pop();
        pthread_mutex_unlock(&queueMutex);

        handleClient((void *)&client_fd);
    }

    return NULL;
}