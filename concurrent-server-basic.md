# 基础的并发服务器  

## 多进程版本  

注意，父进程要在子进程结束时，回收子进程  

```c
#include "wrap.h"
#include <signal.h>
#include <sys/wait.h>

void free_child()
{
    pid_t wpid;
    while (1)
    {
        wpid = waitpid(-1, NULL, WNOHANG);
        if (wpid > 0) {
            printf("child exit, wpid==[%d]\n", wpid);
        } else if (wpid == 0 || wpid == 1) {
            break;
        }
    }
}

int main()
{
    int lfd = Socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serv;
    bzero(&serv, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(8888);
    serv.sin_addr.s_addr = htonl(INADDR_ANY);
    Bind(lfd, (struct sockaddr*)&serv, sizeof(serv));

    Listen(lfd, 128);

    //block SIGCHLD. If all children exit before parent register signal_handler
    //and all children become zombies
    //then parent will not receive signal SIGCHLD
    //and will not call signal_handler to free child's resources
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &mask, NULL);

    pid_t pid;
    int cfd;
    struct sockaddr_in clnt;
    socklen_t clnt_len;
    char clnt_IP[16];

    while (1) {
        // get a new connection
        clnt_len = sizeof(clnt);
        memset(clnt_IP, 0x00, sizeof(clnt_IP));
        cfd = Accept(lfd, (struct sockaddr*)&clnt, &clnt_len);
        printf("clinet: [%s]: [%d]\n", 
            inet_ntop(AF_INET, &clnt.sin_addr.s_addr, clnt_IP, clnt_len),
            ntohs(clnt.sin_port));

        // create a new process
        pid = fork();
        if (pid < 0) {
            perror("fork error");
            exit(1);
        } else if (pid > 0) {
            Close(cfd);

            //register signal handler
            struct sigaction act;
            act.sa_handler = free_child;
            act.sa_flags = 0;
            sigemptyset(&act.sa_mask);
            sigaction(SIGCHLD, &act, NULL);

            //unblock SIGCHLD
            sigprocmask(SIG_UNBLOCK, &mask, NULL);
            //at least one signal handler runs
        } else if (pid == 0) {
            Close(lfd);
            
            int n;
            char buf[MAXLINE];

            while (1) {
                // read message
                n = Read(cfd, buf, sizeof(buf));
                if (n <= 0) {
                    printf("read error or close; ");
                    break;
                }
                printf("port: %d, buf: %s", ntohs(clnt.sin_port), buf);

                // process message
                for (int i = 0; i < n; ++i) {
                    buf[i] = toupper(buf[i]);
                }

                // send message
                Write(cfd, buf, n);
            }
            Close(cfd);
            exit(0);
        }
    }

    Close(lfd);

    return 0;
}
```

## 多线程版本  

下面是基础版的多线程服务器示例。注意几个问题  

1.**子线程不能关闭监听文件描述符(lfd)**，原因是子线程和主线程共享文件描述符，而不是复制的  
2.**主线程也不能关闭连接描述符cfd**，原因是主线程和子线程共享cfd，close(cfd)之后就会关闭，于是报错  
3.子线程不能共享cfd，原因是cfd只会保存最后一个连接描述符，多个子线程会使用同一个连接描述符。因此每个子线程都要使用不同的cfd，可以构造一个struct数组，每个struct是一个子线程独享的信息，里面包括线程号，cfd，客户端IP等  

```c
#include "wrap.h"
#include <pthread.h>

typedef struct info
{
    int cfd;
    int idx;
    pthread_t tid;
    struct sockaddr_in client;
} INFO;

INFO thInfo[1024];

void* thread_work(void* arg)
{
    INFO* p = (INFO* ) arg;
    printf("idx == %d\n", p->idx);

    char IP[16];
    memset(IP, 0x00, sizeof(IP));
    printf("new client:%s, %lu\n", inet_ntop(AF_INET, &(p->client.sin_addr.s_addr), IP, sizeof(IP)), p->tid);

    int n;
    int cfd = p->cfd;
    // struct sockaddr_in client;
    // memcpy(&client, &(p->client), sizeof(p->client));

    char buf[1024];

    while (1)
    {
        //read
        memset(buf, 0x00, sizeof(buf));
        n = Read(cfd, buf, sizeof(buf));
        if (n <= 0) {
            printf("read error or client close\n");
            Close(cfd);
            p->cfd = -1;
            pthread_exit(NULL);
        }
        printf("n == %d, buf == %s\n", n, buf);
        
        // process: to upper
        for (int i = 0; i < n; ++i) {
            buf[i] = toupper(buf[i]);
        }

        //write
        Write(cfd, buf, n);
    }
}

void init_thInfo()
{
    for (int i = 0; i < 1024; ++i) {
        thInfo[i].cfd = -1;
    }
}

int findIndex()
{
    int i;
    for (i = 0; i < 1024; ++i) {
        if (thInfo[i].cfd == -1) {
            break;
        }
    }
    if (i == 1024) {
        return -1;
    }
    return i;
}

int main()
{
    // create socket
    int lfd = Socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));

    // bind
    struct sockaddr_in serv;
    bzero(&serv, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(8888);
    serv.sin_addr.s_addr = htonl(INADDR_ANY);
    Bind(lfd, (struct sockaddr* )&serv, sizeof(serv));

    // listen
    Listen(lfd, 128);

    //init thInfo
    init_thInfo();

    int cfd;
    int idx;
    struct sockaddr_in clnt;
    socklen_t clnt_len;
    int ret = -1;

    while (1)
    {
        clnt_len = sizeof(clnt);
        bzero(&clnt, sizeof(clnt));

        //get a new connection
        cfd = Accept(lfd, (struct sockaddr* )&clnt, &clnt_len);
             
        //find space availale in thInfo
        idx = findIndex();
        if (-1 == idx) {
            Close(cfd);
            continue;
        }

        //assign value in the space chosen above
        thInfo[idx].cfd = cfd;
        thInfo[idx].idx = idx;
        memcpy(&thInfo[idx].client, &clnt, sizeof(clnt));

        //create subthread
        ret = pthread_create(&thInfo[idx].tid, NULL, thread_work, &thInfo[idx]);
        if (-1 == ret) {
            printf("pthread_create failed\n");
            exit(-1);
        }

        // set subthread detach
        pthread_detach(thInfo[idx].tid);
    }

    //close
    Close(lfd);

    return 0;
}
```
