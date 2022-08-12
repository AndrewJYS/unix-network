# 字节序  

## 大端和小端序  

大端字节序也称网络字节序，高位存储低位数字  

下面给出了测试大端小端字节序的例程  

```c++
union
{
    short a;
    char b[sizeof(short)];
} un1;

union
{
    int a;
    char b[sizeof(int)];
} un2;

int main()
{
    cout << sizeof(short) << " " << sizeof(int) << " " << sizeof(long long) << endl;

    un1.a = 0x0102;
    cout << (int)un1.b[0] << " " << (int)un1.b[1] << " " << un1.a << endl;

    un2.a = 0x01020304;
    cout << (int)un2.b[0] << " " << (int)un2.b[1] << " " << (int)un2.b[2] << " " << (int)un2.b[3] << " " << un2.a << endl;

    /*
    2 4 8
    2 1 258
    4 3 2 1 16909060
    因此是小端序，因为低位存储低位数字
    */

    return 0;
}
```

## 字节序转换函数  

```c++
#include <netinet/in.h>

uint16_t htons(uint16_t host16bitvalue);
uint32_t htonl(uint32_t host32bitvalue);
//Both return: value in network byte order

uint16_t ntohs(uint16_t net16bitvalue);
uint32_t ntohl(uint32_t net32bitvalue);
//Both return: value in host byte order
```

其中，h表示host，to表示“到”，n表示network，s表示short，l表示long。比如uint16_t htons(uint16_t host16bitvalue)表示将主机字节序转换成网络字节序，该函数的参数类型是short。如果本来不需要转换，那么函数内部就不会做转换。  

## ip地址转换函数  

```c++
#include <arpa/inet.h>

int inet_pton(int family, const char *strptr, void *addrptr);
//Returns: 1 if OK, 0 if input not a valid presentation format, −1 on error

//其中p表示点分十进制的字符串形式，to表示“到”，n表示network。inet_pton()是将字符串形式的点分十进制IP转换成大端模式的网络IP地址（整型4字节数）
//family：either AF_INET or AF_INET6.（一般用AF_INET）
//strptr：字符串形式的点分十进制IP地址
//addrptr：存放转换后的变量的地址


const char *inet_ntop(int family, const void *addrptr, char *strptr, size_t len);
//Returns: pointer to result if OK, NULL on error

//addrptr：网络的整型IP地址
//strptr：转换后的IP地址，一般为字符串数组  
//len：strptr的长度
```

## 结构体 sockaddr_in  

```c++
struct in_addr 
{
in_addr_t s_addr;           /* 32-bit IPv4 address */
                            /* network byte ordered */
};

struct sockaddr_in 
{
uint8_t sin_len;            /* length of structure (16) */
sa_family_t sin_family;     /* AF_INET */
in_port_t sin_port;         /* 16-bit TCP or UDP port number */
                            /* network byte ordered */

struct in_addr sin_addr;    /* 32-bit IPv4 address */
                            /* network byte ordered */

char sin_zero[8];           /* unused */
};
```

示例如下：  

```c++
struct sockaddr_in serv;
serv.sin_family = AF_INET;
serv.sin_port = htons(8888);
serv.sin_addr.s_addr = htonl(INADDR_ANY);
```

或者可以写成  

```c++
struct sockaddr_in serv;
serv.sin_family = AF_INET;
serv.sin_port = htons(8888);
inet_pton(AF_INET, "点分十进制格式的服务器IP地址", &serv.sin_addr.s_addr);
```
