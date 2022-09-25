#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main()
{
    // create listen socket
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd < 0)
    {
        perror("socket error");
        return -1;
    }

    // bind
    struct sockaddr_in serv;
    bzero(&serv, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(8888);
    serv.sin_addr.s_addr = htonl(INADDR_ANY);
    int bind_ret = bind(lfd, (struct sockaddr*)&serv, sizeof(serv));
    if (bind_ret < 0)
    {
        perror("bind error");
        return -1;
    }

    // listen 
    listen(lfd, 128);

    // accept
    struct sockaddr_in clnt;
    socklen_t len = sizeof(clnt);
    int cfd = accept(lfd, (struct sockaddr*)&clnt, &len);
    printf("lfd== %d, cfd== %d\n", lfd, cfd);

    // read and write
    int n = 0;
    int i = 0;
    char buf[1024];
    while (1)
    {
        // receive 
        memset(buf, 0x00, sizeof(buf));
        n = read(cfd, buf, sizeof(buf));
        if (n <= 0)
        {
            printf("read error or close connection\n");
            break;
        }
        printf("n== %d , buf== %s\n", n, buf);

        //process
        for (i = 0; i < n; i++)
        {
            buf[i] = toupper(buf[i]);
        }

        //send
        write(cfd, buf, n);
    }

    //close
    close(lfd);
    close(cfd);

    return 0;
}