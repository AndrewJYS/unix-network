#ifndef WRAP_H
#define WRAP_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <ctype.h>

#define MAXLINE 4096

void perr_exit(const char* s);

int Socket(int family, int type, int protocol);
int Bind(int fd, const struct sockaddr* sa, socklen_t salenptr);
int Listen(int fd, int backlog);
int Accept(int fd, struct sockaddr* sa, socklen_t* salenptr);
int Connect(int fd, const struct sockaddr* sa, socklen_t salen);
int Close(int fd);

ssize_t Read(int fd, void* ptr, size_t nbytes);
ssize_t Write(int fd, const void* ptr, size_t nbytes);
ssize_t Readn(int fd, void* vptr, size_t n);
ssize_t writen(int fd, const void *vptr, size_t n);
static ssize_t my_read(int fd, char *ptr);
ssize_t readline(int fd, void *vptr, size_t maxlen);
ssize_t readlinebuf(void **vptrptr);
ssize_t Readline(int fd, void *ptr, size_t maxlen);

int tcp4bind(short port, const char* IP);

#endif
