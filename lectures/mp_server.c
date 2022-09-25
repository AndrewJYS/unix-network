#include "wrap.h"
#include <signal.h>
#include <sys/wait.h>

void free_child()
{
    pid_t wpid;
    while (1)
    {
        wpid = waitpid(-1, NULL, WNOHANG);
        if (wpid > 0) {
            printf("child exit, wpid==[%d]\n", wpid);
        } else if (wpid == 0 || wpid == 1) {
            break;
        }
    }
}

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

    //block SIGCHLD. If all children exit before parent register signal_handler
    //and all children become zombies
    //then parent will not receive signal SIGCHLD
    //and will not call signal_handler to free child's resources
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &mask, NULL);

    pid_t pid;
    int cfd;
    struct sockaddr_in clnt;
    socklen_t clnt_len;
    char clnt_IP[16];

    while (1) {
        // get a new connection
        clnt_len = sizeof(clnt);
        memset(clnt_IP, 0x00, sizeof(clnt_IP));
        cfd = Accept(lfd, (struct sockaddr*)&clnt, &clnt_len);
        printf("clinet: [%s]: [%d]\n", 
            inet_ntop(AF_INET, &clnt.sin_addr.s_addr, clnt_IP, clnt_len),
            ntohs(clnt.sin_port));

        // create a new process
        pid = fork();
        if (pid < 0) {
            perror("fork error");
            exit(1);
        } else if (pid > 0) {
            Close(cfd);

            //register signal handler
            struct sigaction act;
            act.sa_handler = free_child;
            act.sa_flags = 0;
            sigemptyset(&act.sa_mask);
            sigaction(SIGCHLD, &act, NULL);

            //unblock SIGCHLD
            sigprocmask(SIG_UNBLOCK, &mask, NULL);
            //at least one signal handler runs
        } else if (pid == 0) {
            Close(lfd);
            
            int n;
            char buf[MAXLINE];

            while (1) {
                // read message
                n = Read(cfd, buf, sizeof(buf));
                if (n <= 0) {
                    printf("read error or close; ");
                    break;
                }
                printf("port: %d, buf: %s", ntohs(clnt.sin_port), buf);

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