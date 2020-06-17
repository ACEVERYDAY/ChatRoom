#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define PORT 8080
#define MAX_BUFF_SIZE 1024

char name[100];
int rsize, wsize;

static int sock;
static char rbuf[1024];
static char wbuf[1024];

static struct sockaddr_in server,client;

int initSocket();
int buildConnection(int option);
void userInterface();
void viewChatRecord();

int main(int argc,char *argv[]) 
{

	// initSocket();
	// buildConnection();
	userInterface();
	return 0;
}


int initSocket() 
{
	int err;
	
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

	// 关闭输出缓冲区，防止printf无法在预期的地方输出
	setvbuf(stdout, NULL, _IONBF, 0);
	
	printf("connect success!\n");
	bzero(rbuf, sizeof(rbuf));
	bzero(name, sizeof(name));
}

int buildConnection(int option) 
{

	printf("username:");
	rsize = read(STDIN_FILENO, rbuf,sizeof(rbuf));
	rbuf[strlen(rbuf)-1] = '\0';
	
	//  读取用户密码 
	printf("password:");
	wsize = read(STDIN_FILENO, wbuf,sizeof(wbuf));
	wbuf[strlen(wbuf)-1] = '\0';	
	
	// 数据格式:  用户名#密码#选项
	strcat(rbuf, "#");
	strcat(rbuf, wbuf);
	strcat(rbuf, "#");
	strcat(rbuf, option==1 ? "1" : "2");
	
	
	send(sock, rbuf, sizeof(name), 0);

	int j = 0;
	int i = 0;
	fd_set rfd;
	FD_ZERO(&rfd);
	FD_SET(STDIN_FILENO, &rfd);
	FD_SET(sock, &rfd);
	while(1) {
		FD_ZERO(&rfd);
		FD_SET(STDIN_FILENO, &rfd);
		FD_SET(sock, &rfd);
		switch(select(sock+1, &rfd, NULL, NULL, NULL)) {
			case -1:
			case 0:
				continue;
			default:
				if(FD_ISSET(STDIN_FILENO, &rfd)) {
					bzero(rbuf, sizeof(rbuf));
					// 读取标准输入，并送往服务器
					rsize = read(STDIN_FILENO, rbuf, sizeof(rbuf));
					if(rsize > 0) {
						send(sock, rbuf, rsize, 0);
					}
				}

				if(FD_ISSET(sock, &rfd)) {
					bzero(rbuf, sizeof(rbuf));
					rsize = recv(sock, rbuf, sizeof(rbuf), 0);
					if(rsize > 0) {
						printf("\033[34m%s\033[30m\n", rbuf);
						if(!strcmp(rbuf, "register successful!")) {
							close(sock);
							return 11;
						}
						else if(!strcmp(rbuf, "user has exist!")) {		
							close(sock);
							return 11;						
						}
						else if(!strcmp(rbuf, "quit")) {
							close(sock);
							return 11;
						}
							
					}
				}
		}
	
	}
	return 1;
}

void viewChatRecord() 
{
	FILE *fp;
	char line[MAX_BUFF_SIZE];
	if(!(fp = fopen("../UserInfo/ChatInfo.txt", "a+"))) {
		perror("file open error!");
		// return FILE_OPEN_ERROR;
	}
	while(fgets(line, MAX_BUFF_SIZE, fp)) {
		printf("%s", line);
	}
	fclose(fp);	
}

// 用户接口界面
void userInterface() 
{
	int option = 0;

	printf("-----------------------------------\n");
	printf("-----------CLIENT MENU-------------\n");
	printf("-----------1. REGISTER-------------\n");
	printf("-----------2. SIGN IN--------------\n");
	printf("-----------3. QUIT-----------------\n");
	printf("-----------------------------------\n");
	printf("after logging in, you can type '#bye' to quit or type '#chat record' to view chat-record.\n");
	
	scanf("%d", &option);
	
	if(option == 3)
		return;
	initSocket();
	buildConnection(option);
	userInterface();	

}
