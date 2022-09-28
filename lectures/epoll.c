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