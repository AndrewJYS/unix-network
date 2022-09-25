#include "wrap.h"

int main()
{
    int lfd, cfd, sockfd;
    int nready;
    int maxfd;
    int n, i;
    char buf[MAXLINE];
    struct sockaddr_in serv, clnt;
    socklen_t len;
    // define fd_set
    fd_set readfds, tmpfds;
    int connfd[FD_SETSIZE]; //valid file descriptor array
    int maxi; //index of max valid file descriptor

    // create socket
    lfd = Socket(AF_INET, SOCK_STREAM, 0);

    // reuse port
    int opt = 1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));

    // bind
    bzero(&serv, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(8888);
    serv.sin_addr.s_addr = htonl(INADDR_ANY);
    Bind(lfd, (struct sockaddr* )&serv, sizeof(serv));

    // listen
    Listen(lfd, 128);

    // clear fd_set
    FD_ZERO(&readfds);
    FD_ZERO(&tmpfds);

    // put lfd into readfds
    FD_SET(lfd, &readfds);

    // init valid file descriptor array
    // -1: this index can be used
    for (i = 0; i < FD_SETSIZE; ++i) {
        connfd[i] = -1;
    }

    maxfd = lfd;
    len = sizeof(struct sockaddr_in);
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
            cfd = Accept(lfd, (struct sockaddr* )&clnt, &len);
            if (cfd < 0) {
                if (errno == ECONNABORTED || errno == EINTR) {
                    continue;
                }
                break;
            }

            // find index available
            for (i = 0; i < FD_SETSIZE; i++) {
                if (connfd[i] == -1) {
                    connfd[i] = cfd;
                    break;
                }
            }
            // if number of connections == FD_SETSIZE
            if (i == FD_SETSIZE) {
                Close(cfd);
                printf("too many connections, i == %d\n", i);
                continue;
            }
            if (i > maxi) {
                maxi = i;
            }

            //print IP of client
            char IP[16];
            memset(IP, 0x00, sizeof(IP));
            printf("new client IP: %s, port: %d\n", inet_ntop(AF_INET, &(clnt.sin_addr.s_addr), IP, sizeof(IP)), ntohs(clnt.sin_port));

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
        for (i = 0; i <= maxi; ++i) {
            sockfd = connfd[i];
            if (-1 == sockfd) {
                continue;
            }

            // judge sockfd whether changed
            if (FD_ISSET(sockfd, &tmpfds)) {
                // read, do not use while(1),
                //because read will block
                memset(buf, 0x00, sizeof(buf));
                n = Read(sockfd, buf, sizeof(buf));
                if (n < 0) {
                    perror("read over");
                    Close(sockfd);

                    // delete sockfd from readfds
                    FD_CLR(sockfd, &readfds);
                    connfd[i] = -1;
                } else if (0 == n) {
                    printf("client is closed\n");
                    Close(sockfd);

                    // delete sockfd from readfds
                    FD_CLR(sockfd, &readfds);
                    connfd[i] = -1;
                } else { 
                    printf("n == %d, buf == %s\n", n, buf);

                    for (int k = 0; k < n; ++k) {
                        buf[k] = toupper(buf[k]);
                    }

                    //write
                    Write(sockfd, buf, n);
                }

                if (--nready == 0) {
                    break;
                }
            }
        }
    }

    Close(lfd);

    return 0;
}