/*
 * @FilePath: /Course/server_file/common/hash_utils.c
 * @Author: ZPY
 * @Date: 2024-08-30 20:08:37
 * @TODO: 
 */
#include "hash_utils.h"
#include <stdio.h>
#include <stdlib.h>

void calculateSHA256(const char *filePath, unsigned char hash[SHA256_DIGEST_LENGTH])
{
    FILE *file = fopen(filePath, "rb");
    if (!file) {
        perror("打开文件失败");
        exit(EXIT_FAILURE);
    }

    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    const int bufSize = BUFFER_SIZE;
    unsigned char *buffer = malloc(bufSize);
    int bytesRead = 0;
    if (!buffer) {
        perror("分配缓冲区失败");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    while ((bytesRead = fread(buffer, 1, bufSize, file)) > 0)
    {
        SHA256_Update(&sha256, buffer, bytesRead);
    }

    SHA256_Final(hash, &sha256);

    fclose(file);
    free(buffer);
}