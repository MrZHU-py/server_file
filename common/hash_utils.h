/*
 * @FilePath: /Course/server_file/common/hash_utils.h
 * @Author: ZPY
 * @Date: 2024-08-30 20:08:44
 * @TODO: 
 */
#ifndef HASH_UTILS_H
#define HASH_UTILS_H

#include <openssl/sha.h>
#include "file_utils.h"

void calculateSHA256(const char *filePath, unsigned char hash[SHA256_DIGEST_LENGTH]);

#endif // HASH_UTILS_H