#include "wrap.h"

int main()
{
    int lfd = Socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serv;
    bzero(&serv, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(8888);
    serv.sin_addr.s_addr = htonl(INADDR_ANY);
    Bind(lfd, (struct sockaddr*)&serv, sizeof(serv));

    Listen(lfd, 128);
    printf("lfd == %d\n", lfd);

    int cfd = -1;
    pid_t pid;
    while (1) {
        // get a new connection
        cfd = Accept(lfd, NULL, NULL);
        printf("cfd == %d\n", cfd);

        // create a new process
        pid = fork();
        if (pid < 0) {
            perror("fork error");
            exit(1);
        } else if (pid > 0) {
            Close(cfd);
        } else if (pid == 0) {
            Close(lfd);
            
            int n;
            char buf[MAXLINE];

            while (1) {
                // read message
                n = Read(cfd, buf, sizeof(buf));
                if (n <= 0) {
                    printf("read error or close\n");
                    break;
                }
        
                // process message
                for (int i = 0; i < n; ++i) {
                    buf[i] = toupper(buf[i]);
                }

                // send message
                Write(cfd, buf, n);
            }
            Close(cfd);
            exit(0);
        }
    }

    Close(lfd);

    return 0;
}

/*
jys@ubuntu:/proc/sys/fs$ ps -ef | grep a.out
jys         7720    7594  0 02:14 pts/0    00:00:00 ./a.out
jys         7731    7720  0 02:14 pts/0    00:00:00 ./a.out
jys         7739    7720  0 02:14 pts/0    00:00:00 ./a.out
jys         7772    7745  0 02:17 pts/3    00:00:00 grep --color=auto a.out
jys@ubuntu:/proc/sys/fs$ ls -l /proc/7720/fd
total 0
lrwx------ 1 jys jys 64 Sep  3 02:17 0 -> /dev/pts/0
lrwx------ 1 jys jys 64 Sep  3 02:17 1 -> /dev/pts/0
lrwx------ 1 jys jys 64 Sep  3 02:17 2 -> /dev/pts/0
lrwx------ 1 jys jys 64 Sep  3 02:17 3 -> 'socket:[140183]'
jys@ubuntu:/proc/sys/fs$ ls -l /proc/7731/fd
total 0
lrwx------ 1 jys jys 64 Sep  3 02:18 0 -> /dev/pts/0
lrwx------ 1 jys jys 64 Sep  3 02:18 1 -> /dev/pts/0
lrwx------ 1 jys jys 64 Sep  3 02:18 2 -> /dev/pts/0
lrwx------ 1 jys jys 64 Sep  3 02:18 4 -> 'socket:[140184]'
jys@ubuntu:/proc/sys/fs$ ls -l /proc/7739/fd
total 0
lrwx------ 1 jys jys 64 Sep  3 02:18 0 -> /dev/pts/0
lrwx------ 1 jys jys 64 Sep  3 02:18 1 -> /dev/pts/0
lrwx------ 1 jys jys 64 Sep  3 02:18 2 -> /dev/pts/0
lrwx------ 1 jys jys 64 Sep  3 02:18 4 -> 'socket:[140226]'
*/