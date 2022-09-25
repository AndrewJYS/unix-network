#include "wrap.h"

void perr_exit(const char* s)
{
    perror(s);
    exit(-1);
}

int Accept(int fd, struct sockaddr* sa, socklen_t* salenptr)
{
    int n;
    
again:
    if ( (n = accept(fd, sa, salenptr)) < 0 ) {
        if ( (ECONNABORTED == errno) || (EINTR == errno))
            goto again;
        else
            perr_exit("accept error");
    }

    return n;
}

int Bind(int fd, const struct sockaddr* sa, socklen_t salenptr)
{
    int n;

    if ( (n = bind(fd, sa, salenptr)) < 0 )
        perr_exit("bind error");

    return n;
}

int Connect(int fd, const struct sockaddr* sa, socklen_t salen)
{
    int n;

    if ( (n = connect(fd, sa, salen)) < 0 )
        perr_exit("connect error");
    
    return n;
}

int Listen(int fd, int backlog)
{
    int n;

    if ( (n = listen(fd, backlog)) < 0 )
        perr_exit("listen error");

    return 0;
}

int Socket(int family, int type, int protocol) 
{
    int n;

    if ( (n = socket(family, type, protocol)) < 0 )
        perr_exit("socket error");
    
    return n;
}

ssize_t Read(int fd, void* ptr, size_t nbytes)
{
    ssize_t n;

again:
    if ( (n = read(fd, ptr, nbytes)) == -1) {
        if (EINTR == errno)
            goto again;
        else    
            return -1;
    }

    return n;
}

ssize_t Write(int fd, const void* ptr, size_t nbytes)
{
    ssize_t n;

again:
    if ( (n = write(fd, ptr, nbytes)) == -1 ) {
        if (EINTR == errno)
            goto again;
        else
            return -1;
    }

    return n;
}

int Close(int fd)
{
    int n;

    if ( (n = close(fd)) == -1 )
        perr_exit("close error");

    return n;
}

ssize_t Readn(int fd, void* vptr, size_t n)
{
    size_t nleft;
    ssize_t nread;
    char* ptr;

    ptr = vptr;
    nleft = n;

    while (nleft > 0) {
        if ( (nread = read(fd, ptr, nleft)) < 0 ) {
            if (errno == EINTR)
                nread = 0;
            else
                return -1;
        } else if (nread == 0)
            break;
        
        nleft -= nread;
        ptr += nread;
    }
    return (n - nleft);
}

ssize_t writen(int fd, const void *vptr, size_t n)
{
	size_t		nleft;
	ssize_t		nwritten;
    const char* ptr;

    ptr = vptr;
	nleft = n;

	while (nleft > 0) {
		if ( (nwritten = write(fd, ptr, nleft)) <= 0 ) {
			if (nwritten < 0 && errno == EINTR)
				nwritten == 0;
			else
				return -1;
		}

		nleft -= nwritten;
		ptr   += nwritten;
	}
	return n;      /* return >= 0 */
}


static int	read_cnt;
static char	*read_ptr;
static char	read_buf[MAXLINE];

static ssize_t my_read(int fd, char *ptr)
{

	if (read_cnt <= 0) {
again:
		if ( (read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0) {
			if (errno == EINTR)
				goto again;
			return(-1);
		} else if (read_cnt == 0)
			return(0);
		read_ptr = read_buf;
	}

	read_cnt--;
	*ptr = *read_ptr++;
	return(1);
}

ssize_t readline(int fd, void *vptr, size_t maxlen)
{
	ssize_t	n, rc;
	char	c, *ptr;

	ptr = vptr;
	for (n = 1; n < maxlen; n++) {
		if ( (rc = my_read(fd, &c)) == 1) {
			*ptr++ = c;
			if (c == '\n')
				break;	/* newline is stored, like fgets() */
		} else if (rc == 0) {
			*ptr = 0;
			return(n - 1);	/* EOF, n - 1 bytes were read */
		} else
			return(-1);		/* error, errno set by read() */
	}

	*ptr = 0;	/* null terminate like fgets() */
	return(n);
}

ssize_t readlinebuf(void **vptrptr)
{
	if (read_cnt)
		*vptrptr = read_ptr;
	return(read_cnt);
}

ssize_t Readline(int fd, void *ptr, size_t maxlen)
{
	ssize_t		n;

	if ( (n = readline(fd, ptr, maxlen)) < 0)
		perr_exit("readline error");
	return(n);
}

int tcp4bind(short port, const char* IP)
{
    struct sockaddr_in serv_addr;
    int lfd = Socket(AF_INET, SOCK_STREAM, 0);
    bzero(&serv_addr, sizeof(serv_addr));
    if (IP == NULL) {
        serv_addr.sin_addr.s_addr = INADDR_ANY;
    } else {
        if (inet_pton(AF_INET, IP, &serv_addr.sin_addr) <= 0) {
            perror(IP);
            exit(1);
        }
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    Bind(lfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    return lfd;
}
