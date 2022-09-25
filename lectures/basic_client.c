#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main()
{
    // create socket
    int cfd = socket(AF_INET, SOCK_STREAM, 0);
    if (cfd < 0)
    {
        perror("socket error");
        return -1;
    }

    // connect server
    struct sockaddr_in serv;
    bzero(&serv, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(8888);
    inet_pton(AF_INET, "127.0.0.1", &serv.sin_addr.s_addr);

    int conn_ret = connect(cfd, (struct sockaddr*)&serv, sizeof(serv));
    if (conn_ret < 0)
    {
        perror("connect error");
        return -1;
    }

    // read and write
    char buf[256];
    int n = 0;
    while (1)
    {
        // read input
        memset(buf, 0x00, sizeof(buf));
        n = read(STDIN_FILENO, buf, sizeof(buf));

        // send 
        write(cfd, buf, n);

        // receive info from server
        memset(buf, 0x00, sizeof(buf));
        n = read(cfd, buf, sizeof(buf));
        if (n <= 0)
        {
            perror("read error or server closed\n");
            break;
        }
        printf("n==%d, buf== %s\n", n, buf);
    }

    //close
    close(cfd);

    return 0;
}