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
                    break;
                }
            }
        }
    }

    Close(lfd);

    return 0;
}