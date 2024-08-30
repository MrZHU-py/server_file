/*
 * @FilePath: /Course/server_file/client/client.c
 * @Author: ZPY
 * @Date: 2024-08-30 20:07:01
 * @TODO: 
 */
#include "client.h"
#include "../common/hash_utils.h"
#include "../common/file_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


void sendFile(const char *filePath, const char *serverIP, int serverPort)
{
    char fileName[FILENAME_SIZE];
    unsigned char fileHash[SHA256_DIGEST_LENGTH];

    // 计算文件的 SHA256 值
    calculateSHA256(filePath, fileHash);

    // 提取文件名
    extractFileName(filePath, fileName);

    // 创建套接字
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("创建套接字失败");
        exit(EXIT_FAILURE);
    }

    // 服务器地址
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(serverPort);
    server_addr.sin_addr.s_addr = inet_addr(serverIP);

    // 连接到服务器
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("连接到服务器失败");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // 发送文件名
    if (write(sock, fileName, FILENAME_SIZE) < 0)
    {
        perror("发送文件名失败");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // 发送文件哈希值
    if (write(sock, fileHash, SHA256_DIGEST_LENGTH) < 0)
    {
        perror("发送文件哈希值失败");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // 接收服务器的响应
    char response[BUFFER_SIZE] = {0};
    if (read(sock, response, sizeof(response) - 1) < 0)
    {
        perror("读取服务器响应失败");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // 移除可能存在的换行符或回车符
    response[strcspn(response, "\r\n")] = 0;

    if (strcmp(response, "EXISTS") == 0)
    {
        printf("文件已存在，上传成功\n");
        close(sock);
        return;
    } 
    else if (strcmp(response, "NOT_EXISTS") == 0)
    {
        printf("文件不存在，开始上传文件内容...\n");
    } 
    else
    {
        fprintf(stderr, "未知响应: %s\n", response);
        close(sock);
        exit(EXIT_FAILURE);
    }

    // 发送文件内容
    FILE *file = fopen(filePath, "rb");
    if (file == NULL) {
        perror("打开文件失败");
        close(sock);
        exit(EXIT_FAILURE);
    }

    char buf[BUFFER_SIZE];
    int size;
    while ((size = fread(buf, 1, sizeof(buf), file)) > 0)
    {
        if (write(sock, buf, size) < 0) {
            perror("发送文件内容失败");
            fclose(file);
            close(sock);
            exit(EXIT_FAILURE);
        }
    }

    fclose(file);
    close(sock);
    printf("文件上传完成\n");
}