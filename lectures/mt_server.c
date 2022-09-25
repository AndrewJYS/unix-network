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