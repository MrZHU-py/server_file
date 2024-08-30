#include "server.h"
#include "redis_utils.h"
#include "thread_pool.h"
#include "log_utils.h"
#include "task_queue.h"
#include "../common/file_utils.h"
#include "../common/hash_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <pthread.h>

#define QUEUE_CAPACITY 100

TaskQueue taskQueue;

int main(int argc, char const *argv[]) {
    // 初始化任务队列
    initTaskQueue(&taskQueue, QUEUE_CAPACITY);

    // 确保文件目录存在
    ensureDirectoryExists(FILE_DIR);

    // 创建套接字
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("创建套接字失败");
        exit(EXIT_FAILURE);
    }

    // 服务器地址
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9000);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // 绑定地址
    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("绑定地址失败");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 监听
    if (listen(server_fd, 9) < 0) {
        perror("监听失败");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("服务器监听中...\n");

    // 创建线程池
    pthread_t threadPool[THREAD_POOL_SIZE];
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        if (pthread_create(&threadPool[i], NULL, workerThread, NULL) != 0) {
            perror("创建线程池线程失败");
            close(server_fd);
            exit(EXIT_FAILURE);
        }
    }

    // 客户端地址
    struct sockaddr_in addr_client;
    socklen_t addr_len = sizeof(addr_client);

    while (1) {
        // 接收请求，建立 TCP 连接，获得了一个新的客户端套接字
        int client_fd = accept(server_fd, (struct sockaddr *)&addr_client, &addr_len);
        if (client_fd < 0) {
            perror("接受客户端连接失败");
            continue;
        }

        // 将客户端套接字添加到任务队列
        enqueueTask(&taskQueue, client_fd);
    }

    // 关闭服务器套接字
    close(server_fd);

    // 等待线程池线程结束
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        pthread_join(threadPool[i], NULL);
    }

    // 销毁任务队列
    destroyTaskQueue(&taskQueue);

    return 0;
}

void *workerThread(void *arg) {
    while (1) {
        int client_fd = dequeueTask(&taskQueue);
        handleClient((void *)&client_fd);
    }

    return NULL;
}

void ensureDirectoryExists(const char *path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        if (mkdir(path, 0700) < 0) {
            perror("创建目录失败");
            exit(EXIT_FAILURE);
        }
    }
}

void *handleClient(void *arg) {
    int client_fd = *((int *)arg);

    redisContext *redis = redisConnect("127.0.0.1", 6379);
    if (redis == NULL || redis->err) {
        if (redis) {
            printf("Redis 连接错误: %s\n", redis->errstr);
            redisFree(redis);
        } else {
            printf("Redis 连接错误: 不能分配 redisContext\n");
        }
        close(client_fd);
        return NULL;
    }

    char fileName[FILENAME_SIZE];
    unsigned char hash[SHA256_DIGEST_LENGTH];

    // 先接收文件名
    if (read(client_fd, fileName, FILENAME_SIZE) <= 0) {
        perror("读取文件名失败");
        close(client_fd);
        redisFree(redis);
        logOperation(fileName, "失败", "读取文件名");
        return NULL;
    }

    // 再接收哈希值
    if (read(client_fd, hash, SHA256_DIGEST_LENGTH) != SHA256_DIGEST_LENGTH) {
        perror("读取哈希值失败");
        close(client_fd);
        redisFree(redis);
        logOperation(fileName, "失败", "读取哈希值");
        return NULL;
    }

    // 检查文件是否已存在
    if (checkFileExistsInRedis(redis, hash, fileName)) {
        // 文件已存在且文件名不同，通知客户端终止传输
        if (write(client_fd, "EXISTS\n", strlen("EXISTS\n")) < 0) {
            perror("通知客户端文件已存在失败");
        }
        // 存储哈希值和文件名到 Redis
        storeFileInRedis(redis, hash, fileName);
        logOperation(fileName, "成功", "已存在相同文件因此创建软链接");
    } else {
        // 文件不存在，通知客户端继续传输
        if (write(client_fd, "NOT_EXISTS\n", strlen("NOT_EXISTS\n")) < 0) {
            perror("通知客户端文件不存在失败");
            close(client_fd);
            redisFree(redis);
            logOperation(fileName, "失败", "通知客户端文件不存在");
            return NULL;
        }

        // 写入文件内容
        char filePath[BUFFER_SIZE];
        snprintf(filePath, sizeof(filePath), "%s%s", FILE_DIR, fileName);

        FILE *f = fopen(filePath, "wb");
        if (f == NULL) {
            perror("文件写入失败");
            close(client_fd);
            redisFree(redis);
            logOperation(fileName, "失败", "文件写入");
            return NULL;
        }

        int size;
        char buf[BUFFER_SIZE];
        while ((size = read(client_fd, buf, sizeof(buf))) > 0) {
            if (fwrite(buf, 1, size, f) != size) {
                perror("写入文件失败");
                fclose(f);
                close(client_fd);
                redisFree(redis);
                logOperation(fileName, "失败", "写入文件");
                return NULL;
            }
        }

        if (size < 0) {
            perror("读取客户端数据失败");
            logOperation(fileName, "失败", "读取客户端数据");
        } else {
            logOperation(fileName, "成功", "新文件");
        }

        fclose(f);

        // 存储哈希值和文件名到 Redis
        storeFileInRedis(redis, hash, fileName);
    }

    close(client_fd);
    redisFree(redis);
    return NULL;
}