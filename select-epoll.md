# select-epoll  

## select  

多路IO技术：同时监听多个文件描述符，将监控的操作交给内核去处理  

```c
#include <sys/select.h>
#include <sys/time.h>

int select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset, const struct timeval *timeout);

// Returns: positive count of ready descriptors, 0 on timeout, −1 on error
```

功能：委托内核监控可读，可写，异常事件  

We start our description of this function with **timeout** argument, which tells the kernel **how long to wait for one of the specified descriptors to become ready**. A timeval structure specifies the number of seconds and microseconds.  

```c
struct timeval {
    long tv_sec; /* seconds */
    long tv_usec; /* microseconds */
};
```

There are three possibilities:  
1.**Wait forever** — Return only when one of the specified descriptors is ready for I/O. For this, we specify the timeout argument as a **null pointer**.  
2.**Wait up to a fixed amount of time**—Return when one of the specified descriptors is ready for I/O, but do not wait beyond the number of seconds and microseconds specified in the timeval structure pointed to by the timeout argument.  
3.**Do not wait at all**—Return immediately after checking the descriptors. This is called polling. To specify this, the timeout argument must point to a timeval structure and the timer value (the number of seconds and microseconds specified by the structure) must be **0**.

The **maxfdp1** argument specifies **the number of descriptors to be tested**. Its value is the **maximum descriptor to be tested plus one** (hence our name of maxfdp1). The descriptors 0, 1, 2, up through and including maxfdp1−1 are tested.  

The three middle arguments, **readset, writeset, and exceptset**, specify the descriptors that we **want the kernel to test for reading, writing, and exception conditions**  

select **modifies** the descriptor sets pointed to by the **readset, writeset, and exceptset** pointers. These **three arguments are value-result arguments**. **When we call the function, we specify the values of the descriptors that we are interested in**, and **on return, the result indicates which descriptors are ready**. We use the FD_ISSET macro on return to test a specific descriptor in an fd_set structure. **Any descriptor that is not ready on return will have its corresponding bit cleared in the descriptor set. To handle this, we turn on all the bits in which we are interested in all the descriptor sets each time we call select**.  

**select uses descriptor sets, typically an array of integers, with each bit in each integer corresponding to a descriptor**. For example, using 32-bit integers, the first element of the array corresponds to descriptors 0 through 31, the second element of the array corresponds to descriptors 32 through 63, and so on. **All the implementation details are irrelevant to the application and are hidden in the fd_set datatype and the following four macros**:  

```c
void FD_ZERO(fd_set *fdset); /* clear all bits in fdset */
void FD_SET(int fd, fd_set *fdset); /* turn on the bit for fd in fdset */
void FD_CLR(int fd, fd_set *fdset); /* turn off the bit for fd in fdset */
int FD_ISSET(int fd, fd_set *fdset); /* is the bit for fd on in fdset ? */
```

example:  

```c
#include "wrap.h"

int main()
{
    // create socket
    int lfd = Socket(AF_INET, SOCK_STREAM, 0);

    // reuse port
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

    // define fd_set
    fd_set readfds;
    fd_set tmpfds;

    // clear fd_set
    FD_ZERO(&readfds);
    FD_ZERO(&tmpfds);

    // put lfd into readfds
    FD_SET(lfd, &readfds);

    int nready;
    int maxfd = lfd;
    int cfd;
    int n;
    char buf[MAXLINE];
    int sockfd;

    while (1) {
        tmpfds = readfds;

        nready = select(maxfd + 1, &tmpfds, NULL, NULL, NULL);
        if (nready < 0) {
            if (errno == EINTR) {
                continue;
            }
            break;
        }
        printf("nready = %d\n", nready);

        // client request for connection
        if (FD_ISSET(lfd, &tmpfds)) {
            // get a new connection
            cfd = Accept(lfd, NULL, NULL);

            // put cfd into readfds
            FD_SET(cfd, &readfds);

            //modify maxfd -> cfd
            if (maxfd < cfd) {
                maxfd = cfd;
            }

            if (--nready == 0) {
                continue;
            }
        }

        // data sent to server
        for (int i = lfd + 1; i < maxfd + 1; ++i) {
            // judge sockfd whether changed
            if (FD_ISSET(i, &tmpfds)) {
                sockfd = i;

                // read, do not use while(1),
                //because read will block
                memset(buf, 0x00, sizeof(buf));
                n = Read(sockfd, buf, sizeof(buf));
                if (n <= 0) {
                    // close connection
                    Close(sockfd);

                    // delete sockfd from readfds
                    FD_CLR(sockfd, &readfds);
                } else {
                    printf("n == %d, buf == %s\n", n, buf);

                    for (int k = 0; k < n; ++k) {
                        buf[k] = toupper(buf[k]);
                    }

                    //write
                    Write(sockfd, buf, n);
                }

                if (--nready == 0) {
                    continue;
                }
            }
        }
    }

    Close(lfd);

    return 0;
}
```

## epoll

```c
#include <sys/epoll.h>

int epoll_create(int size);

//epoll_create创建一棵epoll树，返回一个树的根节点，size必须传一个大于0的数 
//返回一个文件描述符，这个文件描述符就表示epoll树的根节点
```

```c
#include <sys/epoll.h>

int epoll_ctl(int epfd, int op, int fd, struct epoll_event * event);

//将fd上epoll树，从树上删除和修改
//成功时返回0, 失败时返回－1 。
```

epfd：epoll树的树根节点。  
op：用千指定监视对象的添加、删除或更改等操作。  
fd：需要注册的监视对象文件描述符。  
event：监视对象的事件类型。  

op的选项如下：  
EPOLL_CTL_ADD: 上树操作。  
EPOLL_CTL_DEL: 从树上删除节点。  
EPOLL_CTL_MOD: 修改。  

epoll_event的结构如下：  

```c
typedef union epoll_data {
    void        *ptr;
    int          fd;
    uint32_t     u32;
    uint64_t     u64;
} epoll_data_t;

struct epoll_event {
    uint32_t     events;      /* Epoll events */
    epoll_data_t data;        /* User data variable */
};
```

epoll_event中events的选项如下：  
EPOLLIN：可读事件  
EPOLLOUT：可写事件  
EPOLLERR：异常事件  

epoll_ctl的示例如下：  

```c
struct epoll_event ev;
ev.events = EPOLLIN;
ev.data.fd = fd;
epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
```

```c
#include <sys/epoll.h>

int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);

//委托内核监控epoll树的事件节点
//成功时返回发生事件的文件描述符数，失败时返回－1
```

epfd：epoll树根  
events：传出参数，结构体数组  
maxevents：events数组大小  
timeout:  

+ \-1 表示阻塞  
+ 0 表示不阻塞
+ \>0 表示阻塞超时时长  

示例如下：  

```c
#include "wrap.h"
#include <ctype.h>
#include <sys/epoll.h>

int main()
{
    int ret;
    int n;
    int i;
    int k;
    int nready;
    int lfd;
    int cfd;
    int sockfd;
    char buf[1024];
    socklen_t socklen;
    struct sockaddr_in servaddr;
    struct epoll_event ev;
    struct epoll_event events[1024];

    // create socket
    lfd = Socket(AF_INET, SOCK_STREAM, 0);

    // reuse port
    int opt = 1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));

    // bind
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(8888);
    Bind(lfd, (struct sockaddr* )&servaddr, sizeof(struct sockaddr_in));

    //listen
    Listen(lfd, 128);

    // create an epoll tree
    int epfd;
    epfd = epoll_create(1024);
    if (epfd < 0) 
    {
        perror("create epoll error");
        return -1;
    }

    // put lfd onto epoll tree
    ev.data.fd = lfd;
    ev.events = EPOLLIN;
    epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev);

    while (1) 
    {
        nready = epoll_wait(epfd, events, 1024, -1);
        if (nready < 0)
        {
            perror("epoll_wait error");
            if (errno == EINTR)
            {
                continue;
            }
            break;
        }

        for (i = 0; i < nready; i++)
        {
            sockfd = events[i].data.fd;

            // new client connection
            if (sockfd == lfd)
            {
                cfd = Accept(lfd, NULL, NULL);
                // put cfd into epoll tree
                ev.data.fd = cfd;
                ev.events = EPOLLIN;
                epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);
                continue;
            }

            // data from client
            memset(buf, 0x00, sizeof(buf));
            n = Read(sockfd, buf, sizeof(buf));
            if (n <= 0)
            {
                Close(sockfd);
                //delete sockfd from epoll tree
                epoll_ctl(epfd, EPOLL_CTL_DEL, sockfd, NULL);
            }
            else
            {
                for (k = 0; k < n; ++k)
                {
                    buf[k] = toupper(buf[k]);
                }
                Write(sockfd, buf, n);
            }
        }
    }

    Close(lfd);
    return 0;
}
```

### epoll的水平模式和边缘模式  

水平触发模式(LT)：高电平代表1，只要缓冲区有数据，就一直通知，epoll默认是LT模式  
边缘触发模式：电平有变化就代表1，缓冲区中有数据只会通知一次，之后再有数据才会通知（如果读数据的时候没有读完，则剩余的数据不会再次通知，直到有新的数据到来）  

```c
if (sockfd == lfd)
{
    cfd = Accept(lfd, NULL, NULL);
    // put cfd into epoll tree
    ev.data.fd = cfd;
    ev.events = EPOLLIN | EPOLLET;
    epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);
    continue;
}
```
