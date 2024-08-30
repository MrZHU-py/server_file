#include "log_utils.h"
#include <stdio.h>
#include <time.h>

#define LOG_FILE "server_file_log.txt"

void logOperation(const char *fileName, const char *status, const char *operationType) {
    FILE *logFile = fopen(LOG_FILE, "a");
    if (logFile == NULL) {
        perror("打开日志文件失败");
        return;
    }

    time_t now = time(NULL);
    char timeStr[20];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localtime(&now));

    fprintf(logFile, "[%s] 文件名: %s, 状态: %s, 操作类型: %s\n", timeStr, fileName, status, operationType);
    fclose(logFile);
}