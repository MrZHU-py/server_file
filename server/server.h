#ifndef SERVER_H
#define SERVER_H

void *handleClient(void *arg);
void *workerThread(void *arg);
void ensureDirectoryExists(const char *path);

#endif // SERVER_H