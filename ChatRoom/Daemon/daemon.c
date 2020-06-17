#include <time.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/param.h>
#include <signal.h>
#include <stdlib.h>
#define PORT 8080
#define MAX_BUFF_SIZE 1024

FILE *tp;
int i, j, err;
char name[100];
int rsize, wsize;

static int sock;
static char rbuf[MAX_BUFF_SIZE];
static char wbuf[MAX_BUFF_SIZE];

static struct sockaddr_in server, client;

int initSocket();
void initDaemon();
void listen2Server();
char *getRealMessage(char *message);

int main() 
{
	initDaemon();
	initSocket();
	listen2Server();
	return 0;
}

// 根据报文格式: 长度#报文 解析字符串
char *getRealMessage(char *message) 
{
	// 读取#前字符串代表的长度
	int i = 0, length = 0, index = 0;
	char real_str[MAX_BUFF_SIZE];	
	while(message[index] != '#') {
		length = length * 10 + (message[index++]-'0');
	}
	index += 1;
	for(i = 0;i < length;i++) {
		real_str[i] = message[index+i];
	}
	real_str[length] = '\0';
	strcpy(message, real_str);
	return message;
}

// 初始化套接字信息
int initSocket() 
{

	// 创建套接字->初始化套接字(ip+端口号+未使用的填充位)->建立连接 
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock == -1) {
		perror("socket");
		return 1;
	}
	
	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr("127.0.0.1");

	server.sin_port = htons(PORT);
	err = connect(sock, (struct sockaddr *)&server, sizeof(struct sockaddr));
	if(err == -1) {
		perror("connect");
		return 2;
	}
	bzero(rbuf, sizeof(rbuf));
	bzero(name, sizeof(name));
	
	// 首次输入用户名并显示--------送往服务器做验证 
	strcpy(rbuf, "5b08410da6d1703#4b8795d7bc45f58ec");
	send(sock, rbuf, sizeof(name), 0);
	return 1;
}

// 初始化守护进程
void initDaemon() 
{
	// 通用操作
	pid_t child1, child2;
	int i;
	child1 = fork();
	if(child1 > 0) {
		exit(0);
	}else if (child1 < 0) {
		perror("fail to create child-process!\n");
		exit(1);
	}
	setsid();
	chdir("../SystemLog/");
	umask(0);
	for(i = 0; i < NOFILE; ++i)
		close(i);

	return;
}

// 持续接收
void listen2Server() 
{
	char tmpc[MAX_BUFF_SIZE];
	while(1) {
		rsize = recv(sock, rbuf, sizeof(rbuf), 0);
		// 成功接收
		if(rsize > 0) {
			// success!
			// printf("rbuf: %s\n", rbuf);
	       	if(!(tp = fopen("daemon.log", "a+"))) {
       			perror("open daemon.log error");
            			exit(EXIT_FAILURE);
        		}
        		strcpy(tmpc, getRealMessage(rbuf));
        		fprintf(tp, tmpc);
        		fflush(tp);			
		}
	}
	fclose(tp);
}
