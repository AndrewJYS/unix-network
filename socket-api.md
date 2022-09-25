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

## 套接字选项  

### setsockopt

```c
#include <sys/socket.h>

int setsockopt(int sockfd, int level, int option, const void *val, socklen_t len);

// Returns: 0 if OK, −1 on error
```

sockfd是一个监听描述字  
The **level** argument identifies the protocol to which the option applies. If the option is a generic socket-level option, then level is set to **SOL_SOCKET**. Otherwise, level is set to
the number of the protocol that controls the option. Examples are IPPROTO_TCP for TCP options and IPPROTO_IP for IP options.  

We can set **option SO_REUSEADDR**, then **reuse addresses in bind if \*val is nonzero**.

**The val argument points to a data structure or an integer, depending on the option**. Some options are on/off switches. **If the integer is nonzero, then the option is enabled**. If the integer is zero, then the option is disabled. **The len argument specifies the size of the object to which val points**.

```c
int lfd = Socket(AF_INET, SOCK_STREAM, 0);

int val = 1;
setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int));
```

## 半关闭：shutdown()  

The normal way to terminate a network connection is to call the close function. But, there are two limitations with close that can be avoided with shutdown:
1.**close decrements the descriptor’s reference count and closes the socket only if the count reaches 0**.  
2.close terminates both directions of data transfer, reading and writing. Since a TCP connection is full-duplex, there are times when we want to tell the other end that we have finished sending, even though that end might have more data to send us.  

```c
#include <sys/socket.h>

int shutdown(int sockfd, int howto);

// Returns: 0 if OK, −1 on error
```

The action of the function depends on the value of the howto argument.

|howto |description |
:-:|:-:
|**SHUT_RD**| **The read half of the connection is closed**—**No more data can be received on the socket and any data currently in the socket receive buffer is discarded**. The process can no longer issue any of the read functions on the socket. **Any data received after this call for a TCP socket is acknowledged and then silently discarded**.|
|**SHUT_WR** |**The write half of the connection is closed**—In the case of TCP, this is called a half-close. Any data currently in the socket send buffer will be sent, followed by TCP’s normal connection termination sequence. As we mentioned earlier, this closing of the write half is done regardless of whether or not the socket descriptor’s reference count is currently greater than 0. **The process can no longer issue any of the write functions on the socket.**|
|**SHUT_RDWR** | The read half and the write half of the connection are both closed — **This is equivalent to calling shutdown twice: first with SHUT_RD and then with SHUT_WR**.  

## 心跳包  

心跳包是主要用于检测长连接是否正常的字符串。如果发现异常就要重新连接。下面举一个自定义心跳包的例子  

```md
1.服务A给B发送心跳数据AAAA，服务B收到AAAA之后，给A回复BBBB，此时A收到BBBB之后，认为连接正常  
2.加入A连续发送了3~5次之后，仍然没有收到B的回复，则认为连接异常，异常之后，A应该重建连接：先close原来的连接，然后再重新connect  
```

如何让心跳数据和正常业务数据不混淆？

```md
双方可以协商协议，如4个字节长度+具体数据
如果发送心跳数据应该发送0004AAAA
如果发送业务数据，应该是00101234567890

双方接受数据时先收4个字节的报头数据，然后计算长度，若最后计算长度为4，且数据为AAAA，则认为是心跳数据，则B服务会组织应答数据给A:0004BBBB
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

假如server中的 listen(lfd, 128);后边加上 sleep(30); 那么在server睡眠的30s内使用netstat命令，输出如下  

```shell
(Not all processes could be identified, non-owned process info
 will not be shown, you would have to be root to see it all.)
tcp        1      0 0.0.0.0:8888            0.0.0.0:*               LISTEN      11620/./server1     
tcp        0      0 127.0.0.1:42280         127.0.0.1:8888          ESTABLISHED 11623/./client1     
tcp        0      0 127.0.0.1:8888          127.0.0.1:42280         ESTABLISHED -            
```

可以看到，此时连接已经建立，因此accept不是新建一个连接，而是从已连接队列中拿出一个连接  

## references  

Advanced Programming in the UNIX Environment, W. Richard Stevens / Stephen A. Rago, 3rd edition, Chapter 16  
Unix Network Programming, Volume 1: The Sockets Networking API, W.Richard Stevens / Bill Fenner / Andrew M. Rudoff, 3rd edition, Chapter 3, 4, 6, 7  
