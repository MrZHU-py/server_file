/*
 * @FilePath: /Course/server_file/common/file_utils.c
 * @Author: ZPY
 * @Date: 2024-08-30 20:08:16
 * @TODO: 
 */
#include "file_utils.h"
#include <string.h>

void extractFileName(const char *filePath, char *fileName)
{
    const char *baseName = strrchr(filePath, '/');
    if (baseName)
    {
        strncpy(fileName, baseName + 1, FILENAME_SIZE);
    }
    else
    {
        strncpy(fileName, filePath, FILENAME_SIZE);
    }
}