#ifndef _SERVER_H_
#define _SERVER_H_

#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT               8080
#define MAX_CLIENT         10
#define MAX_BUFF_SIZE      1024
#define FILE_OPEN_ERROR    -1
#define USR_HAS_EXIST      1
#define READ_SUCCESS       1
#define WRITE_SUCCESS      1
#define VARIFY_SUCCESS     2
#define MAX_CHAT_RSIZE     65536

struct client{
	int c_fd;               // 文件描述符 
	char name[50];          // 当前用户名 
	int online;             // 用户在线状态(非客户端) 
};

FILE *tp;                      // 守护进程写入的日志文件指针
int max_sock, len, daemon_fd;  // daemon_fp 为记录守护进程的文件描述符

static int sock;
static char rbuf[MAX_BUFF_SIZE];
static char wbuf[MAX_BUFF_SIZE];
static char pbuf[MAX_BUFF_SIZE];
static char temp_msg[MAX_BUFF_SIZE];
static char login_msg[MAX_BUFF_SIZE];


static struct sockaddr_in server, client;
static struct client cli[MAX_CLIENT];

int initSocket();
char *getChatRecord();
int viewSystemLog();
int isDaemonProcess(char *str1, char *str2);
int registerUser(char *usr_name, char *usr_pwd);
int varifyLogin(char *check_name, char *check_pwd, int flag);

char *getLocalTime();
char *itoa(int length);

void userInterface();
#endif
