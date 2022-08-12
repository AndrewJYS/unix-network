#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 10
void error_handling(char *message);

int main(int argc, char* argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
	char message[BUF_SIZE];
	char mes[BUF_SIZE];
	int str_len;
	
	if(argc != 3){
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	
	sock = socket(PF_INET, SOCK_STREAM, 0);
	if(sock == -1)
		error_handling("socket() error");
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));
		
	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) 
		error_handling("connect() error!");
	else
		printf("Connected..........");
	
	while (1) {
		fputs("Input message(Q to quit): ", stdout); 
		//fputs: output one line once a time
		fgets(message, BUF_SIZE, stdin); 
		//fgets: input one line once a time
		//if BUF_SIZE = 10, input "abcdefghij"
		//then call fgets() twice, firstly get "abcdefghi\0", secondly get "j\n\0"
		//strlen(): length not include '\0', but include '\n'
		//strlen("abcdefghi\0") = 9, strlen("j\n\0") = 2

		if (!strcmp(message, "q\n") || !strcmp(message, "Q\n"))
			break;
		
		write(sock, message, strlen(message)); 
		// strlen(message) is at most BUF_SIZE - 1 bytes, because '\0\ at the end
		// after write(), message has been flushed, but read() not
		// so if we use another buffer for read(), say, mes
		// mes will keep information from read() last time 
		// unless information is overwritten

		printf("length of sent message: %ld\n", strlen(message));

		str_len = read(sock, mes, BUF_SIZE - 1);
		printf("length of received message: %d, %ld\n", str_len, strlen(mes));
		//mes[str_len] = 0;
		
		printf("Message from server: %s \n", mes);  
	}

	close(sock);
	return 0;
}

void error_handling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
