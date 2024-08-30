/*
 * @FilePath: /Course/server_file/server/task_queue.h
 * @Author: ZPY
 * @Date: 2024-08-30 20:32:01
 * @TODO: 
 */
#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H

#include <pthread.h>

typedef struct TaskQueue {
    int *tasks;
    int capacity;
    int size;
    int front;
    int rear;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} TaskQueue;

void initTaskQueue(TaskQueue *queue, int capacity);
void destroyTaskQueue(TaskQueue *queue);
void enqueueTask(TaskQueue *queue, int task);
int dequeueTask(TaskQueue *queue);
int isTaskQueueEmpty(TaskQueue *queue);

#endif // TASK_QUEUE_H