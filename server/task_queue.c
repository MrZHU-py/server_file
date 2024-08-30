/*
 * @FilePath: /Course/server_file/server/task_queue.c
 * @Author: ZPY
 * @Date: 2024-08-30 20:32:31
 * @TODO: 
 */
#include "task_queue.h"
#include <stdlib.h>
#include <stdio.h>

void initTaskQueue(TaskQueue *queue, int capacity) {
    queue->tasks = (int *)malloc(sizeof(int) * capacity);
    if (queue->tasks == NULL) {
        perror("分配任务队列内存失败");
        exit(EXIT_FAILURE);
    }
    queue->capacity = capacity;
    queue->size = 0;
    queue->front = 0;
    queue->rear = -1;
    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->cond, NULL);
}

void destroyTaskQueue(TaskQueue *queue) {
    free(queue->tasks);
    pthread_mutex_destroy(&queue->mutex);
    pthread_cond_destroy(&queue->cond);
}

void enqueueTask(TaskQueue *queue, int task) {
    pthread_mutex_lock(&queue->mutex);
    if (queue->size == queue->capacity) {
        perror("任务队列已满");
        pthread_mutex_unlock(&queue->mutex);
        return;
    }
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->tasks[queue->rear] = task;
    queue->size++;
    pthread_cond_signal(&queue->cond);
    pthread_mutex_unlock(&queue->mutex);
}

int dequeueTask(TaskQueue *queue) {
    pthread_mutex_lock(&queue->mutex);
    while (queue->size == 0) {
        pthread_cond_wait(&queue->cond, &queue->mutex);
    }
    int task = queue->tasks[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size--;
    pthread_mutex_unlock(&queue->mutex);
    return task;
}

int isTaskQueueEmpty(TaskQueue *queue) {
    pthread_mutex_lock(&queue->mutex);
    int isEmpty = (queue->size == 0);
    pthread_mutex_unlock(&queue->mutex);
    return isEmpty;
}