#include "redis_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *hashToHexString(const unsigned char *hash, char *outputBuffer) {
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
    }
    return outputBuffer;
}

int checkFileExistsInRedis(redisContext *redis, const unsigned char *hash, const char *fileName) {
    char hexHash[SHA256_DIGEST_LENGTH * 2 + 1];
    hashToHexString(hash, hexHash);

    redisReply *reply = redisCommand(redis, "EXISTS %s", hexHash);
    int exists = reply->integer;
    freeReplyObject(reply);

    return exists;
}

void storeFileInRedis(redisContext *redis, const unsigned char *hash, const char *fileName) {
    char hexHash[SHA256_DIGEST_LENGTH * 2 + 1];
    hashToHexString(hash, hexHash);

    // 检查是否存在相同哈希值的文件
    redisReply *reply = redisCommand(redis, "EXISTS %s", hexHash);
    int hashExists = reply->integer;
    freeReplyObject(reply);

    if (hashExists) {
        // 获取已存在的文件名
        reply = redisCommand(redis, "GET %s", hexHash);
        char *existingFileName = strdup(reply->str);
        freeReplyObject(reply);

        // 改变当前工作目录到 FILE_DIR
        if (chdir(FILE_DIR) < 0) {
            perror("无法进入目标目录");
            return;
        }

        // 创建软链接
        if (symlink(existingFileName, fileName) < 0) {
            perror("创建软链接失败");
        }

        free(existingFileName);
    } else {
        // 添加新哈希和文件名到 Redis
        reply = redisCommand(redis, "SET %s %s", hexHash, fileName);
        if (reply == NULL) {
            printf("存储文件信息到 Redis 失败\n");
        }
        freeReplyObject(reply);
    }
}