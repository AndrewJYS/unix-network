# socket API  

## socket  

```c
#include <sys/socket.h>

int socket(int family, int type, int protocol);

//Returns: non-negative descriptor if OK, −1 on error
//创建socket
```

family: 协议版本  
    AF_INET: IPv4 protocols  
    AF_INET6: IPv6 protocols  
    AF_LOCAL: Unix domain protocols  

type：协议类型  
    SOCK_STREAM: stream socket，TCP  
    SOCK_DGRAM: datagram socket，UDP  

protocol: 一般填0，表示使用对应类型的默认协议  

## bind  

```c
#include <sys/socket.h>

int bind(int sockfd, const struct sockaddr_in *myaddr, socklen_t addrlen);

//Returns: 0 if OK, −1 on error
//将socket文件描述符和IP,PORT绑定
```

sockfd：调用socket函数返回的文件描述符  
myaddr：本地服务器的IP地址和PORT，该结构体示例如下：  
addrlen：myaddr变量的占用的内存大小  

## listen  

```c
#include <sys/socket.h>

int listen(int sockfd, int backlog);

//Returns: 0 if OK, −1 on error
//将套接字由主动态变为被动监听状态
```

sockfd：调用socket函数返回的文件描述符  
backlog：同时请求连接的最大个数（请求连接指还未建立连接）  

## accept  

```c
#include <sys/socket.h>

int accept(int sockfd, struct sockaddr *cliaddr, socklen_t *addrlen);

//Returns: non-negative descriptor if OK, −1 on error
//获得一个连接，若当前没有连接，则会阻塞等待。如果有连接，就创建用于通信的套接字  
```

sockfd：调用socket函数返回的文件描述符  
cliaddr：传出参数，保存客户端的地址信息  
addrlen：传入传出参数，cliaddr变量所占内存空间大小  

## connect  

```c
#include <sys/socket.h>

int connect(int sockfd, const struct sockaddr *servaddr, socklen_t addrlen);

//Returns: 0 if OK, −1 on error
//连接服务器
```

sockfd：调用socket函数返回的文件描述符  
servaddr：服务器的地址信息  
addrlen：servaddr变量的大小  

## 数据传输函数  

```c
#include <unistd.h>

ssize_t read(int fd, void *buf, size_t nbytes);
//Returns: number of bytes read, 0 if end of file, −1 on error

ssize_t write(int fd, const void *buf, size_t nbytes);
//Returns: number of bytes written if OK, −1 on error
```

```c
#include <sys/socket.h>

ssize_t recv(int sockfd, void *buff, size_t nbytes, int flags);
ssize_t send(int sockfd, const void *buff, size_t nbytes, int flags);
//Both return: number of bytes read or written if OK, −1 on error
//flag可以直接填0 
```

## server示例  

```c
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
    int cfd = accept(lfd, NULL, NULL);
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
```

## client示例  

```c
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
```

## netstat命令  

测试过程中可以使用netstat命令查看监听状态和连接状态  
a表示显示所有  
n表示显示的时候以数字的形式显示  
p表示显示进程信息（进程名和进程ID）  

示例（只显示端口8888的相关信息）：  

```shell
netstat -anp | grep 8888   
```

若上述代码的server和client连接成功，则显示如下  

```shell
(Not all processes could be identified, non-owned process info
 will not be shown, you would have to be root to see it all.)
tcp        0      0 0.0.0.0:8888            0.0.0.0:*               LISTEN      11377/./server1     
tcp        0      0 127.0.0.1:42276         127.0.0.1:8888          ESTABLISHED 11409/./client1     
tcp        0      0 127.0.0.1:8888          127.0.0.1:42276         ESTABLISHED 11377/./server1  
```
