#ifndef REDIS_UTILS_H
#define REDIS_UTILS_H

#include <hiredis/hiredis.h>
#include <openssl/sha.h>

int checkFileExistsInRedis(redisContext *redis, const unsigned char *hash, const char *fileName);
void storeFileInRedis(redisContext *redis, const unsigned char *hash, const char *fileName);
char *hashToHexString(const unsigned char *hash, char *outputBuffer);

#endif // REDIS_UTILS_H